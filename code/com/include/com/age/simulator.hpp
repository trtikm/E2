#ifndef COM_AGE_SIMULATOR_HPP_INCLUDED
#   define COM_AGE_SIMULATOR_HPP_INCLUDED

#   include <com/simulation_context.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>
#   include <gfx/gui/text_box.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <boost/property_tree/ptree.hpp>
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

struct Serialisable;

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


void nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState, float_32_bit const  round_seconds);
void renderRound(SimulatorState const& simulatorState, gfx::draw_state& dstate);
std::shared_ptr<PlayerInteraction const> getPlayerInteraction(SimulatorState const& simulatorState, ObjectId const  playerId);

// save and load methods below return an error message on error and empty string on success.

std::string save(
        WorldState const& worldState,
        MessagePassing const& messaging,
        SimulatorState const& simulatorState,
        boost::property_tree::ptree&  ptree
        );
std::string load(
        boost::property_tree::ptree const&  ptree,
        WorldState& worldState,
        MessagePassing& messaging,
        SimulatorState& simulatorState
        );


}}

namespace  com { namespace age {


struct Serialisable
{
    // Both methods return an error message on error and empty string on success.

    virtual std::string save(boost::property_tree::ptree&  ptree) const { return ""; }
    virtual std::string load(boost::property_tree::ptree const&  ptree) const { return ""; }
};


struct  WorldState : public Serialisable
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


struct  MessagePassing : public Serialisable
{
    struct MessageInfo
    {
        ObjectId sender;
        Message message;
        ObjectId location; // Optional: A place from where the sender sent the message.
    };

    using MessageVector = std::vector<MessageInfo>;

    MessagePassing() : Serialisable(), m_messages(), m_pendingMessages() {}
    virtual ~MessagePassing() {}

    virtual std::unique_ptr<MessagePassing>  clone() const;

    bool hasMessage(ObjectId const& receiver, Message const& message) const;
    bool hasMessage(ObjectId const& receiver, ObjectId const& sender, Message const& message) const;
    MessageVector const& messages(ObjectId const& receiver) const;
    void sendMessage(ObjectId const& sender, ObjectId const& receiver, Message const& message, ObjectId const& location = invalid_object_id);

private:
    friend void com::age::nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState, float_32_bit const  round_seconds);

    void deliverMessages(ObjectId const& receiver);

    std::unordered_map<ObjectId, MessageVector> m_messages;
    std::unordered_map<ObjectId, MessageVector> m_pendingMessages;
    static MessageVector const emptyMessages;
};


struct  ActorState : public Serialisable
{
    ActorState(ObjectId const&  self_id) : Serialisable(), m_self_id(self_id) {}
    virtual ~ActorState() {}

    ObjectId  self_id() const { return m_self_id; }

    virtual std::unique_ptr<ActorState>  clone() const { return std::make_unique<ActorState>(self_id()); }

private:
    ObjectId  m_self_id;
};


struct  ActionContext : public Serialisable
{
    virtual ~ActionContext() {}
};


struct  RobotAction : public Serialisable
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


struct  RobotScore : public Serialisable
{
    virtual ~RobotScore() {}

    virtual float compute(
            ActorState const& robotState,
            WorldState const& worldState,
            MessagePassing const& messaging,
            SimulatorState const& simulatorState
            ) const = 0;
};


struct  PlayerAction : public Serialisable
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

struct  PlayerInteraction : public Serialisable
{
    struct  TextLocalisation
    {
        TextLocalisation(boost::property_tree::ptree const&  ptree);
        std::string object(ObjectId const&  id) const { return get(objects, id); }
        std::string feature(Feature const&  f) const { return get(features, f); }
        std::string relation(Relation const&  r) const { return get(relations, r); }
        std::string message(Message const&  msg) const { return get(messages, msg); }
        std::string token(std::string const&  tkn) const { return get(tokens, tkn); }
    private:
        static inline std::string get(std::unordered_map<std::string, std::string> const& data, std::string const&  key)
        {
            auto const it = data.find(key);
            return it == data.end() ? key : it->second;
        }
        std::unordered_map<std::string, std::string> objects;
        std::unordered_map<std::string, std::string> features;
        std::unordered_map<std::string, std::string> relations;
        std::unordered_map<std::string, std::string> messages;
        std::unordered_map<std::string, std::string> tokens;
    };

    using ActionAndContext = std::pair<std::shared_ptr<PlayerAction const>, std::unique_ptr<ActionContext const> >;
    using ActionsVector = std::vector<ActionAndContext>;

    struct ActionSelectionProps
    {
        ActionSelectionProps() : actionInfos(), chosenActionIndex(std::numeric_limits<std::size_t>::max()) {}
        ActionsVector actionInfos;
        std::size_t chosenActionIndex;
    };

    PlayerInteraction(
            std::shared_ptr<TextLocalisation const> const  localisation_,
            simulation_context_ptr const ctx_,
            osi::keyboard_props const& keyboard_,
            osi::mouse_props const& mouse_,
            osi::window_props const& window_,
            std::shared_ptr<gfx::font_mono_props const> const  font_,
            std::shared_ptr<gfx::viewport const> const  viewport_
            );
    virtual ~PlayerInteraction() {}

    std::shared_ptr<TextLocalisation const> const  getTextLocalisation() const { return textLocalisation; }

    virtual void onNextPlayerActivated() {}

    // Collects data from the scene to be later rendered for the player.
    virtual void processScene(float_32_bit const round_seconds);

    // This method is used by the default implementation of method processScene above.
    // You are supposed to convert the scene into a text info for the player. The player
    // is then supposed to choose his action based on that info. Actions must be referenced
    // by numbers in range [1,...,actionSelectionProps->actionInfos.size()].
    virtual void processSceneToText(float_32_bit const round_seconds, std::stringstream& sstr) {}

    // Until the player reads the message return false. Then return true.
    virtual bool processMessage(MessagePassing::MessageInfo const&  message, float_32_bit const round_seconds);

    // When readActionIndex() below returns an invalid action index, then this method is called
    // to render corresponding error message. Player's input is passed to the method.
    // Until the player reads the message return false. Then return true.
    // The defaut implementation uses token "input_error_wrong_player_action_index" to get the
    // corresponding text from 'textLocalisation' module. Pressing ENTER means player read the message.
    virtual bool processMessageInvalidActionIndex(std::string const& input, float_32_bit const round_seconds);

    // Returns std::numeric_limits<std::size_t>::max() until no action is selected.
    // Once an action is selected, its index in range [0,...,actionSelectionProps->actionInfos.size()-1UL] is returned.
    // The default implementation reads keyboard input as numbers in range [1,...,actionSelectionProps->actionInfos.size()]
    // and then subtracts 1.
    virtual std::size_t readActionIndex(float_32_bit const round_seconds);

    // Renders data processed by methods above to the screen.
    virtual void render(gfx::draw_state& dstate) const;

protected:
    std::unique_ptr<ActionSelectionProps> actionSelectionProps;
    ActorState const* playerState;
    WorldState const* worldState;
    MessagePassing::MessageVector messages;
    SimulatorState const* simulatorState;

    std::shared_ptr<TextLocalisation const> const  textLocalisation;

    simulation_context_ptr ctx;
    osi::keyboard_props const* keyboard;
    osi::mouse_props const* mouse;
    osi::window_props const* window;

    gfx::gui::text_box  textBox;
    std::string inputText;
    bool hasInvalidInput;

private:
    friend void com::age::nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState, float_32_bit const  round_seconds);

    bool update(float_32_bit const round_seconds);

    void onNextPlayerActivated(
            com::age::ActorState const* playerState_,
            com::age::WorldState const* worldState_,
            MessagePassing::MessageVector const& messages_,
            com::age::SimulatorState const* simulatorState_
            );
};


struct  SimulatorState final : public Serialisable
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
    friend void com::age::nextRound(WorldState& worldState, MessagePassing& messaging, SimulatorState& simulatorState, float_32_bit const  round_seconds);
    friend void com::age::renderRound(SimulatorState const& simulatorState, gfx::draw_state& dstate);
    friend std::shared_ptr<PlayerInteraction const> com::age::getPlayerInteraction(SimulatorState const& simulatorState, ObjectId const  playerId);

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
