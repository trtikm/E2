#ifndef COM_AGE_SIMULATOR_HPP_INCLUDED
#   define COM_AGE_SIMULATOR_HPP_INCLUDED

#   include <com/simulation_context.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <unordered_map>
#   include <set>
#   include <vector>
#   include <list>
#   include <string>
#   include <memory>
#   include <limits>
#   include <functional>
#   include <algorithm>
#   include <typeinfo>
#   include <typeindex>

namespace  com { namespace age {


using  ObjectId = std::string;
using  Feature = std::string;
using  Relation = std::string;
using  Message = std::string;

extern ObjectId const invalid_object_id;


struct  WorldState;

struct  MessagePassing;

struct  ActionContext;
struct  ActionEffectDescription;

struct  RobotState;
struct  RobotAction;
struct  RobotScore;

struct  PlayerAction;
struct  PlayerInteraction;

struct  SimulatorState;
struct  Simulator;

void nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState);
void nextPlayerInteraction(
        SimulatorState const& simulatorState,
        simulation_context_ptr const ctx,
        osi::keyboard_props const& keyboard,
        osi::mouse_props const& mouse,
        osi::window_props const& window
        );


}}

namespace  com { namespace age {


struct  WorldState
{
    using EntitiesSet = std::set<ObjectId>;
    using FeaturesSet = std::set<Feature>;

    WorldState();
    virtual ~WorldState() {}

    virtual std::unique_ptr<WorldState>  clone() const;

    void insertEntity(ObjectId const& entity);
    void eraseEntity(ObjectId const& entity);
    bool hasEntity(ObjectId const& entity) const;
    EntitiesSet const& entities(Feature const& feature) const;

    void insertFeature(ObjectId const& entity, Feature const& feature);
    void eraseFeature(ObjectId const& entity, Feature const& feature);
    bool hasFeature(ObjectId const& entity, Feature const& feature) const;
    FeaturesSet const& features(ObjectId const& entity) const;

    void insertRelation(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity);
    void eraseRelation(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity);
    void eraseRelation(Relation const& relation, ObjectId const& srcEntity);
    void eraseRelation(Relation const& relation);
    bool hasRelation(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity) const;
    EntitiesSet const& mappedBy(Relation const& relation, ObjectId const& srcEntity) const;
    EntitiesSet const& mappedTo(Relation const& relation, ObjectId const& dstEntity) const;

    void insertRelationFeature(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, Feature const& feature);
    void eraseRelationFeature(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, Feature const& feature);
    void eraseRelationFeatures(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity);
    void eraseRelationFeatures(Relation const& relation, ObjectId const& srcEntity);
    bool hasRelationFeature(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, Feature const& feature) const;
    FeaturesSet const& relationFeatures(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity) const;

    void insertRelationLabel(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, ObjectId const& label);
    void eraseRelationLabel(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, ObjectId const& label);
    void eraseRelationLabels(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity);
    void eraseRelationLabels(Relation const& relation, ObjectId const& srcEntity);
    bool hasRelationLabel(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, ObjectId const& label) const;
    EntitiesSet const& relationLabels(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity) const;

private:
    using FromEntitiesToFeaturesMap = std::unordered_map<ObjectId, FeaturesSet>;
    using FromFeaturesToEntitiesMap = std::unordered_map<Feature, EntitiesSet>;
    using RelationsMap = std::unordered_map<Relation, std::unordered_map<ObjectId, EntitiesSet> >;
    using RelationFeaturesMap = std::unordered_map<Relation, std::unordered_map<std::pair<ObjectId, ObjectId>, FeaturesSet> >;
    using RelationLabelsMap = std::unordered_map<Relation, std::unordered_map<std::pair<ObjectId, ObjectId>, EntitiesSet> >;

    FromEntitiesToFeaturesMap fromEntitiesToFeatures;
    FromFeaturesToEntitiesMap fromFeaturesToEntities;
    RelationsMap relationsForward;
    RelationsMap relationsInverse;
    RelationFeaturesMap fromRelationToFeatures;
    RelationFeaturesMap fromRelationToLabels;
    static EntitiesSet const emptyEntities;
    static FeaturesSet const emptyFeatures;
};


struct  MessagePassing
{
    struct MessageInfo
    {
        ObjectId sender;
        Message message;
        ObjectId location; // Optional: A place from where the sender sent the message.
    };

    using MessageVector = std::vector<MessageInfo>;

    MessagePassing() : m_messages(), m_pendingMessages() {}
    virtual ~MessagePassing() {}

    virtual std::unique_ptr<MessagePassing>  clone() const;

    bool hasMessage(ObjectId const& receiver, Message const& message) const;
    bool hasMessage(ObjectId const& receiver, ObjectId const& sender, Message const& message) const;
    MessageVector const& messages(ObjectId const& receiver) const;
    void sendMessage(ObjectId const& sender, ObjectId const& receiver, Message const& message, ObjectId const& location = invalid_object_id);

private:

    friend void com::age::nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState);

    void deliverMessages(ObjectId const& receiver);

    std::unordered_map<ObjectId, MessageVector> m_messages;
    std::unordered_map<ObjectId, MessageVector> m_pendingMessages;
    static MessageVector const emptyMessages;
};


struct  ActorState
{
    ActorState(ObjectId const&  self_id) : m_self_id(self_id) {}
    virtual ~ActorState() {}

    ObjectId  self_id() const { return m_self_id; }

    virtual std::unique_ptr<ActorState>  clone() const { return std::make_unique<ActorState>(self_id()); }

private:
    ObjectId  m_self_id;
};


struct  ActionContext
{
    virtual ~ActionContext() {}
};


struct  RobotAction
{
    virtual ~RobotAction() {}

    virtual void contexts(
            ActorState const& robotState,
            WorldState const& worldState,
            MessagePassing const& messaging,
            SimulatorState const& simulatorState,
            std::function<void(std::unique_ptr<ActionContext>)> const&  contextAcceptor
            ) const = 0;

    virtual void apply(
            ActionContext const& context,
            ActorState& robotState,
            WorldState& worldState,
            MessagePassing& messaging,
            SimulatorState& simulatorState
            ) const = 0;
};


struct  RobotScore
{
    virtual ~RobotScore() {}

    virtual float compute(
            ActorState const& robotState,
            WorldState const& worldState,
            MessagePassing const& messaging,
            SimulatorState const& simulatorState
            ) const = 0;
};


struct  PlayerAction
{
    virtual ~PlayerAction() {}

    virtual void contexts(
            ActorState const& playerState,
            WorldState const& worldState,
            MessagePassing const& messaging,
            SimulatorState const& simulatorState,
            std::function<void(std::unique_ptr<ActionContext>)> const&  contextAcceptor
            ) const = 0;

    virtual void apply(
            ActionContext const& context,
            ActorState& playerState,
            WorldState& worldState,
            MessagePassing& messaging,
            SimulatorState& simulatorState
            ) const = 0;
};

struct  PlayerInteraction
{
    using ActionAndContext = std::pair<std::shared_ptr<PlayerAction const>, std::unique_ptr<ActionContext const> >;
    using ActionsVector = std::vector<ActionAndContext>;

    struct ActionSelectionProps
    {
        ActionSelectionProps() : actionInfos(), chosenActionIndex(std::numeric_limits<std::size_t>::max()) {}
        ActionsVector actionInfos;
        std::size_t chosenActionIndex;
    };

    PlayerInteraction();
    virtual ~PlayerInteraction() {}

    virtual void onNextPlayerActivated() {}
    virtual void chooseAction() = 0;
    virtual void reportNoActionAvailable() = 0;

protected:
    std::unique_ptr<ActionSelectionProps> actionSelectionProps;
    ActorState const* playerState;
    WorldState const* worldState;
    MessagePassing::MessageVector const* messages;
    SimulatorState const* simulatorState;

    simulation_context_ptr ctx;
    osi::keyboard_props const* keyboard;
    osi::mouse_props const* mouse;
    osi::window_props const* window;

private:
    friend void com::age::nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState);
    friend void com::age::nextPlayerInteraction(
            SimulatorState const& simulatorState,
            simulation_context_ptr const ctx,
            osi::keyboard_props const& keyboard,
            osi::mouse_props const& mouse,
            osi::window_props const& window
            );

    void onNextPlayerActivated(
            com::age::ActorState const* playerState_,
            com::age::WorldState const* worldState_,
            MessagePassing::MessageVector const* messages_,
            com::age::SimulatorState const* simulatorState_
            );
    void setContextAndDevices(
            simulation_context_ptr const ctx_,
            osi::keyboard_props const* keyboard_,
            osi::mouse_props const* mouse_,
            osi::window_props const* window_
            );
};


struct  SimulatorState final
{
    SimulatorState();

    void insertRobot(std::unique_ptr<ActorState> robotState, std::shared_ptr<RobotScore> const robotScore, unsigned int const maxSearchDepth);
    void insertPlayer(std::unique_ptr<ActorState> playerState, std::shared_ptr<PlayerInteraction> const interaction);
    void eraseRobot(ObjectId const& robot);
    void erasePlayer(ObjectId const& player);

    void insertRobotAction(ObjectId const& robot, std::shared_ptr<RobotAction> const action);
    void insertPlayerAction(ObjectId const& player, std::shared_ptr<PlayerAction> const action);
    void eraseRobotAction(ObjectId const& robot, std::type_info const& actionType);
    void erasePlayerAction(ObjectId const& player, std::type_info const& actionType);

    void setActiveActor(ObjectId const& actor);
    ObjectId const& activeActor() const;
    void activateNextActor();

private:

    friend void com::age::nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState);
    friend void com::age::nextPlayerInteraction(
            SimulatorState const& simulatorState,
            simulation_context_ptr const ctx,
            osi::keyboard_props const& keyboard,
            osi::mouse_props const& mouse,
            osi::window_props const& window
            );

    void activatePreviousActor();

    std::unique_ptr<SimulatorState> clone() const;

    struct ActorRobot
    {
        ActorRobot(
                std::unique_ptr<ActorState> robotState_,
                std::shared_ptr<RobotScore> const robotScore_,
                unsigned int const maxSearchDepth_,
                std::unordered_map<std::type_index, std::shared_ptr<RobotAction> > const& actions_ = {}
                );

        ActorRobot clone() const { return ActorRobot(robotState->clone(), robotScore, maxSearchDepth, actions); }

        std::unique_ptr<ActorState>  robotState;
        std::shared_ptr<RobotScore>  robotScore;
        unsigned int  maxSearchDepth;
        std::unordered_map<std::type_index, std::shared_ptr<RobotAction> > actions;
    };

    struct ActorPlayer
    {
        ActorPlayer(
                std::unique_ptr<ActorState> playerState_,
                std::shared_ptr<PlayerInteraction> const interaction_,
                std::unordered_map<std::type_index, std::shared_ptr<PlayerAction> > const& actions_ = {}
                );

        ActorPlayer clone() const { return ActorPlayer(playerState->clone(), interaction, actions); }

        std::unique_ptr<ActorState> playerState;
        std::shared_ptr<PlayerInteraction>  interaction;
        std::unordered_map<std::type_index, std::shared_ptr<PlayerAction> > actions;
    };

    std::unordered_map<ObjectId, ActorRobot>  robots;
    std::unordered_map<ObjectId, ActorPlayer>  players;
    std::list<ObjectId> order;
    std::list<ObjectId>::iterator activeActorIterator;
};


}}

#endif
