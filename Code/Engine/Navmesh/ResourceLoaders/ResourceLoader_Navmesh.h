#pragma once

#include "Engine/_Module/API.h"
#include "Base/Resource/ResourceLoader.h"

//-------------------------------------------------------------------------

namespace EE::Navmesh
{
    class NavmeshLoader : public Resource::ResourceLoader
    {
    public:

        NavmeshLoader();

    private:

        virtual bool Load( ResourceID const& resourceID, FileSystem::Path const& resourcePath, Resource::ResourceRecord* pResourceRecord, Serialization::BinaryInputArchive& archive ) const override final;
        virtual void UnloadInternal( ResourceID const& resourceID, Resource::ResourceRecord* pResourceRecord ) const override final;
    };
}