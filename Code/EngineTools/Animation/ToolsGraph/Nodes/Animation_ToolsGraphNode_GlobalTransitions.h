#pragma once
#include "Animation_ToolsGraphNode_Transition.h"

//-------------------------------------------------------------------------

namespace EE::Animation
{
    class GlobalTransitionConduitToolsNode;

    // The result node for a global transition
    //-------------------------------------------------------------------------
    // There is always one of these nodes created for every state in the parent state machine

    class GlobalTransitionToolsNode final : public TransitionToolsNode
    {
        friend GlobalTransitionConduitToolsNode;
        friend StateMachineGraph;
        EE_REFLECT_TYPE( GlobalTransitionToolsNode );

    public:

        inline UUID const& GetEndStateID() const { return m_stateID; }

        virtual char const* GetTypeName() const override { return "Global Transition"; }
        virtual char const* GetCategory() const override { return "State Machine"; }
        virtual bool IsUserCreatable() const override { return false; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::ValueTree ); }

    private:

        EE_REFLECT( Hidden );
        UUID m_stateID;
    };

    // The global transition node present in state machine graphs
    //-------------------------------------------------------------------------

    class GlobalTransitionConduitToolsNode final : public NodeGraph::StateMachineNode
    {
        EE_REFLECT_TYPE( GlobalTransitionConduitToolsNode );

    public:

        GlobalTransitionConduitToolsNode();

        bool HasGlobalTransitionForState( UUID const& stateID ) const;

        void UpdateTransitionNodes();

        void UpdateStateMapping( THashMap<UUID, UUID> const& IDMapping );

    private:

        virtual bool IsVisible() const override { return false; }
        virtual Color GetTitleBarColor() const override { return Colors::OrangeRed; }
        virtual char const* GetTypeName() const override { return "Global Transitions"; }
        virtual bool IsUserCreatable() const override { return false; }
        virtual void OnShowNode() override;
    };
}