#include "ResourceCompiler_PhysicsCollisionMesh.h"
#include "EngineTools/Physics/ResourceDescriptors/ResourceDescriptor_PhysicsCollisionMesh.h"
#include "EngineTools/Import/Importer.h"
#include "EngineTools/Import/ImportedMesh.h"
#include "Engine/Physics/PhysicsCollisionMesh.h"
#include "Engine/Physics/Physics.h"
#include "Base/FileSystem/FileSystem.h"
#include "Base/Serialization/BinarySerialization.h"

//-------------------------------------------------------------------------

using namespace physx;

//-------------------------------------------------------------------------

namespace EE::Physics
{
    class PhysxMemoryStream : public PxOutputStream
    {
    public:

        PhysxMemoryStream( Blob& destination ) : m_buffer( destination ) {}

    private:

        virtual PxU32 write( const void* src, PxU32 count ) override final
        {
            size_t originalSize = m_buffer.size();
            m_buffer.resize( originalSize + count );
            memcpy( &m_buffer[originalSize], src, count );
            return count;
        }

    private:

        Blob& m_buffer;
    };

    //-------------------------------------------------------------------------

    CollisionMeshCompiler::CollisionMeshCompiler()
        : Resource::Compiler( "CollisionMeshCompiler" )
    {
        AddOutputType<CollisionMesh>();
    }

    Resource::CompilationResult CollisionMeshCompiler::Compile( Resource::CompileContext const& ctx ) const
    {
        PhysicsCollisionMeshResourceDescriptor resourceDescriptor;
        if ( !Resource::ResourceDescriptor::TryReadFromFile( *m_pTypeRegistry, ctx.m_inputFilePath, resourceDescriptor ) )
        {
            return Error( "Failed to read resource descriptor from input file: %s", ctx.m_inputFilePath.c_str() );
        }

        if ( Math::IsNearZero( resourceDescriptor.m_scale.m_x ) || Math::IsNearZero( resourceDescriptor.m_scale.m_y ) || Math::IsNearZero( resourceDescriptor.m_scale.m_z ) )
        {
            return Error( "Zero Scale is not allowed!", ctx.m_inputFilePath.c_str() );
        }

        // Create collision mesh
        //-------------------------------------------------------------------------

        CollisionMesh physicsMesh;
        physicsMesh.m_collisionSettings = resourceDescriptor.m_collisionSettings;

        // Read raw mesh data
        //-------------------------------------------------------------------------

        FileSystem::Path meshFilePath;
        if ( !ConvertDataPathToFilePath( resourceDescriptor.m_sourcePath, meshFilePath ) )
        {
            return Error( "Invalid source data path: %s", resourceDescriptor.m_sourcePath.c_str() );
        }

        Import::ReaderContext readerCtx = { [this]( char const* pString ) { Warning( pString ); }, [this] ( char const* pString ) { Error( pString ); } };
        TUniquePtr<Import::ImportedMesh> pImportedMesh = Import::ReadStaticMesh( readerCtx, meshFilePath, resourceDescriptor.m_meshesToInclude );
        if ( pImportedMesh == nullptr )
        {
            return Error( "Failed to read mesh from source file" );
        }

        EE_ASSERT( pImportedMesh->IsValid() );
        pImportedMesh->ApplyScale( resourceDescriptor.m_scale );

        // Reflect FBX data into physics format
        //-------------------------------------------------------------------------

        Blob cookedMeshData;

        if ( resourceDescriptor.m_isConvexMesh )
        {
            if ( !CookConvexMeshData( *pImportedMesh, cookedMeshData ) )
            {
                return CompilationFailed( ctx );
            }

            physicsMesh.m_isConvexMesh = true;
        }
        else
        {
            if ( !CookTriangleMeshData( *pImportedMesh, cookedMeshData ) )
            {
                return CompilationFailed( ctx );
            }

            physicsMesh.m_isConvexMesh = false;
        }

        // Set Materials
        //-------------------------------------------------------------------------
        // For now just use the default material until we have a proper DCC physics pipeline

        if ( physicsMesh.IsConvexMesh() )
        {
            physicsMesh.m_materialIDs.emplace_back( MaterialSettings::s_defaultID );
        }
        else // One material per geometry section
        {
            for ( auto i = 0; i < pImportedMesh->GetNumGeometrySections(); i++ )
            {
                physicsMesh.m_materialIDs.emplace_back( MaterialSettings::s_defaultID );
            }
        }

        // Serialize
        //-------------------------------------------------------------------------

        Resource::ResourceHeader hdr( CollisionMesh::s_version, CollisionMesh::GetStaticResourceTypeID(), ctx.m_sourceResourceHash, ctx.m_advancedUpToDateHash );
        Serialization::BinaryOutputArchive archive;
        archive << hdr << physicsMesh << cookedMeshData;

        if ( archive.WriteToFile( ctx.m_outputFilePath ) )
        {
            if ( pImportedMesh->HasWarnings() )
            {
                return CompilationSucceededWithWarnings( ctx );
            }
            else
            {
                return CompilationSucceeded( ctx );
            }
        }
        else
        {
            return CompilationFailed( ctx );
        }
    }

    bool CollisionMeshCompiler::CookTriangleMeshData( Import::ImportedMesh const& ImportedMesh, Blob& outCookedData ) const
    {
        PX::Allocator allocator;
        PX::UserErrorCallback errorCallback;

        PxTolerancesScale tolerancesScale;
        tolerancesScale.length = Constants::s_lengthScale;
        tolerancesScale.speed = Constants::s_speedScale;

        auto pFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, allocator, errorCallback );
        auto pCooking = PxCreateCooking( PX_PHYSICS_VERSION, *pFoundation, PxCookingParams( tolerancesScale ) );
        if ( pCooking == nullptr )
        {
            pFoundation->release();
            pFoundation = nullptr;
            Error( "PxCreateCooking failed!" );
            return false;
        }

        // Reflect raw mesh into physx
        //-------------------------------------------------------------------------
        // PhysX meshes require counterclockwise winding

        PxTriangleMeshDesc meshDesc;
        meshDesc.points.stride = sizeof( float ) * 3;
        meshDesc.triangles.stride = sizeof( uint32_t ) * 3;
        meshDesc.materialIndices.stride = sizeof( PxMaterialTableIndex );

        TVector<Float3> vertexData;
        TVector<uint32_t> indexData;
        TVector<PxMaterialTableIndex> materialIndexData;

        PxMaterialTableIndex materialIdx = 0;
        for ( auto const& geometrySection : ImportedMesh.GetGeometrySections() )
        {
            // Add the verts
            for ( auto const& vert : geometrySection.m_vertices )
            {
                vertexData.push_back( vert.m_position );
            }

            // Add the indices - taking into account offset from previously added verts
            for ( auto idx : geometrySection.m_indices )
            {
                indexData.push_back( meshDesc.points.count + idx );
            }

            // Add material indices
            uint32_t const numTriangles = geometrySection.GetNumTriangles();
            for ( auto triIdx = 0u; triIdx < geometrySection.GetNumTriangles(); triIdx++ )
            {
                materialIndexData.emplace_back( materialIdx );
            }

            meshDesc.points.count += (uint32_t) geometrySection.m_vertices.size();
            meshDesc.triangles.count += numTriangles;
            materialIdx++;
        }

        meshDesc.points.data = vertexData.data();
        meshDesc.triangles.data = indexData.data();
        meshDesc.materialIndices.data = materialIndexData.data();

        //-------------------------------------------------------------------------

        outCookedData.clear();
        PhysxMemoryStream stream( outCookedData );
        PxTriangleMeshCookingResult::Enum result;
        pCooking->cookTriangleMesh( meshDesc, stream, &result );

        //-------------------------------------------------------------------------

        pCooking->release();
        pFoundation->release();
        pFoundation = nullptr;

        //-------------------------------------------------------------------------

        if ( result == PxTriangleMeshCookingResult::eLARGE_TRIANGLE )
        {
            Error( "Triangle mesh cooking failed - Large triangle detected" );
            return false;
        }
        else if ( result == PxTriangleMeshCookingResult::eFAILURE )
        {
            Error( "Triangle mesh cooking failed!" );
            return false;
        }

        return true;
    }

    bool CollisionMeshCompiler::CookConvexMeshData( Import::ImportedMesh const& ImportedMesh, Blob& outCookedData ) const
    {
        PX::Allocator allocator;
        PX::UserErrorCallback errorCallback;

        PxTolerancesScale tolerancesScale;
        tolerancesScale.length = Constants::s_lengthScale;
        tolerancesScale.speed = Constants::s_speedScale;

        auto pFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, allocator, errorCallback );
        auto pCooking = PxCreateCooking( PX_PHYSICS_VERSION, *pFoundation, PxCookingParams( tolerancesScale ) );
        if ( pCooking == nullptr )
        {
            pFoundation->release();
            pFoundation = nullptr;
            Error( "PxCreateCooking failed!" );
            return false;
        }

        // Reflect raw mesh into a physx convex mesh
        //-------------------------------------------------------------------------

        TVector<Float3> vertexData;
        TVector<uint32_t> indexData;
        uint32_t indexOffset = 0;

        for ( auto const& geometrySection : ImportedMesh.GetGeometrySections() )
        {
            // Add the verts
            for ( auto const& vert : geometrySection.m_vertices )
            {
                vertexData.push_back( vert.m_position );
            }

            // Add the indices - taking into account offset from previously added verts
            for ( auto idx : geometrySection.m_indices )
            {
                indexData.push_back( indexOffset + idx );
            }

            indexOffset += (uint32_t) geometrySection.m_vertices.size();
        }

        //-------------------------------------------------------------------------

        PxConvexMeshDesc convexMeshDesc;
        convexMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
        convexMeshDesc.points.count = (uint32_t) vertexData.size();
        convexMeshDesc.points.stride = sizeof( float ) * 3;
        convexMeshDesc.points.data = vertexData.data();

        convexMeshDesc.indices.count = (uint32_t) indexData.size();
        convexMeshDesc.indices.stride = sizeof( uint32_t ) * 3;
        convexMeshDesc.indices.data = indexData.data();

        //-------------------------------------------------------------------------

        outCookedData.clear();
        PhysxMemoryStream stream( outCookedData );

        PxConvexMeshCookingResult::Enum result;
        pCooking->cookConvexMesh( convexMeshDesc, stream, &result );

        //-------------------------------------------------------------------------

        pCooking->release();
        pFoundation->release();
        pFoundation = nullptr;

        //-------------------------------------------------------------------------

        if ( result == PxConvexMeshCookingResult::eZERO_AREA_TEST_FAILED )
        {
            Error( "Convex mesh cooking failed - Zero area test failed" );
            return false;
        }
        else if ( result == PxConvexMeshCookingResult::ePOLYGONS_LIMIT_REACHED )
        {
            Error( "Convex mesh cooking failed - Polygon limit reached" );
            return false;
        }
        else if ( result == PxConvexMeshCookingResult::eFAILURE )
        {
            Error( "Convex mesh cooking failed!" );
            return false;
        }

        return true;
    }
}
