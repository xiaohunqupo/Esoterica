#include "Animation_ToolsGraphNode_AnimationClip.h"
#include "Engine/Animation/Graph/Nodes/Animation_RuntimeGraphNode_AnimationClip.h"
#include "EngineTools/Animation/ToolsGraph/Animation_ToolsGraph_Compilation.h"

//-------------------------------------------------------------------------

namespace EE::Animation
{
    AnimationClipToolsNode::AnimationClipToolsNode()
        :VariationDataToolsNode()
    {
        CreateOutputPin( "Pose", GraphValueType::Pose );
        CreateInputPin( "Play In Reverse", GraphValueType::Bool );
        CreateInputPin( "Reset Time", GraphValueType::Bool );

        m_defaultVariationData.CreateInstance( GetVariationDataTypeInfo() );
    }

    int16_t AnimationClipToolsNode::Compile( GraphCompilationContext& context ) const
    {
        AnimationClipNode::Definition* pDefinition = nullptr;
        NodeCompilationState const state = context.GetDefinition<AnimationClipNode>( this, pDefinition );
        if ( state == NodeCompilationState::NeedCompilation )
        {
            auto pShouldPlayInReverseNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pShouldPlayInReverseNode != nullptr )
            {
                auto compiledNodeIdx = pShouldPlayInReverseNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pDefinition->m_playInReverseValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }

            //-------------------------------------------------------------------------

            auto pResetTimeNode = GetConnectedInputNode<FlowToolsNode>( 1 );
            if ( pResetTimeNode != nullptr )
            {
                auto compiledNodeIdx = pResetTimeNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pDefinition->m_resetTimeValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }

            //-------------------------------------------------------------------------

            auto pData = GetResolvedVariationDataAs<Data>( context.GetVariationHierarchy(), context.GetVariationID() );
            pDefinition->m_dataSlotIdx = context.RegisterResource( pData->m_animClip.GetResourceID() );
            pDefinition->m_speedMultiplier = pData->m_speedMultiplier;
            pDefinition->m_startSyncEventOffset = pData->m_startSyncEventOffset;
            pDefinition->m_sampleRootMotion = m_sampleRootMotion;
            pDefinition->m_allowLooping = m_allowLooping;
        }
        return pDefinition->m_nodeIdx;
    }
}