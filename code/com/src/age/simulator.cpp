#include <com/age/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/algorithm/string.hpp>
#include <unordered_set>
#include <deque>

namespace  com { namespace age {


ObjectId const invalid_object_id = "";


WorldState::WorldState()
    : Serialisable()
    , fromEntitiesToFeatures()
    , fromFeaturesToEntities()
    , relationsForward()
    , relationsInverse()
    , fromRelationToFeatures()
    , fromRelationToLabels()
{}


std::unique_ptr<WorldState> WorldState::clone() const
{
    std::unique_ptr<WorldState>  result = std::make_unique<WorldState>();
    *result = *this;
    return result;
}


void WorldState::insertEntity(ObjectId const& entity)
{
    fromEntitiesToFeatures.insert({ entity, {} });
}


void WorldState::eraseEntity(ObjectId const& entity)
{
    FeaturesSet const feats = features(entity);
    for (Feature const& feature : feats)
        eraseFeature(entity, feature);
    fromEntitiesToFeatures.erase(entity);

    {
        std::vector<RelationFeaturesMap::iterator> relErase;
        for (auto rit = fromRelationToFeatures.begin(); rit != fromRelationToFeatures.end(); ++rit)
        {
            std::vector<RelationFeaturesMap::mapped_type::iterator> toErase;
            for (auto it = rit->second.begin(); it != rit->second.end(); ++it)
                if (it->first.first == entity || it->first.second == entity)
                    toErase.push_back(it);
            for (auto it : toErase)
                rit->second.erase(it);
            if (rit->second.empty())
                relErase.push_back(rit);
        }
        for (auto it : relErase)
            fromRelationToFeatures.erase(it);
    }

    {
        std::vector<RelationLabelsMap::iterator> relErase;
        for (auto rit = fromRelationToLabels.begin(); rit != fromRelationToLabels.end(); ++rit)
        {
            std::vector<RelationLabelsMap::mapped_type::iterator> toErase;
            for (auto it = rit->second.begin(); it != rit->second.end(); ++it)
                if (it->first.first == entity || it->first.second == entity)
                    toErase.push_back(it);
            for (auto it : toErase)
                rit->second.erase(it);
            if (rit->second.empty())
                relErase.push_back(rit);
        }
        for (auto it : relErase)
            fromRelationToLabels.erase(it);
    }


    auto const doErase = [&entity](RelationsMap& relations) {
        std::unordered_set<Relation> relErase;
        for (auto& rel_and_map : relations)
        {
            rel_and_map.second.erase(entity);
            EntitiesSet entErase;
            for (auto& ent_and_map : rel_and_map.second)
            {
                ent_and_map.second.erase(entity);
                if (ent_and_map.second.empty())
                    entErase.insert(ent_and_map.first);
            }
            for (ObjectId const& ent : entErase)
                rel_and_map.second.erase(ent);
            if (rel_and_map.second.empty())
                relErase.insert(rel_and_map.first);
        }
        for (Relation const& rel : relErase)
            relations.erase(rel);
    };
    doErase(relationsForward);
    doErase(relationsInverse);
}


bool WorldState::hasEntity(ObjectId const& entity) const
{
    return fromEntitiesToFeatures.count(entity) != 0UL;
}


WorldState::EntitiesSet const& WorldState::entities(Feature const& feature) const
{
    auto const it = fromFeaturesToEntities.find(feature);
    return it == fromFeaturesToEntities.end() ? emptyEntities : it->second;
}


void WorldState::insertFeature(ObjectId const& entity, Feature const& feature)
{
    fromEntitiesToFeatures[entity].insert(feature);
}


void WorldState::eraseFeature(ObjectId const& entity, Feature const& feature)
{
    auto eit = fromEntitiesToFeatures.find(entity);
    if (eit != fromEntitiesToFeatures.end()) eit->second.erase(feature);
    auto fit = fromFeaturesToEntities.find(feature);
    if (fit != fromFeaturesToEntities.end()) fit->second.erase(entity);
}


bool WorldState::hasFeature(ObjectId const& entity, Feature const& feature) const
{
    return features(entity).count(feature) != 0UL;
}


WorldState::FeaturesSet const& WorldState::features(ObjectId const& entity) const
{
    auto const it = fromEntitiesToFeatures.find(entity);
    return it == fromEntitiesToFeatures.end() ? emptyFeatures : it->second;
}


void WorldState::insertRelation(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity)
{
    relationsForward[relation][srcEntity].insert(dstEntity);
    relationsInverse[relation][dstEntity].insert(srcEntity);
}


void WorldState::eraseRelation(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity)
{
    eraseRelationFeatures(relation, srcEntity, dstEntity);
    eraseRelationLabels(relation, srcEntity, dstEntity);

    auto const doErase = [&relation](RelationsMap& relations, ObjectId const& srcEntity, ObjectId const& dstEntity) {
        auto rit = relations.find(relation);
        if (rit != relations.end())
        {
            auto sit = rit->second.find(srcEntity);
            if (sit != rit->second.end())
            {
                sit->second.erase(dstEntity);
                if (sit->second.empty())
                {
                    rit->second.erase(sit);
                    if (rit->second.empty())
                        relations.erase(rit);
                }
            }
        }
    };
    doErase(relationsForward, srcEntity, dstEntity);
    doErase(relationsInverse, dstEntity, srcEntity);
}


void WorldState::eraseRelation(Relation const& relation, ObjectId const& srcEntity)
{
    eraseRelationFeatures(relation, srcEntity);
    eraseRelationLabels(relation, srcEntity);

    {
        auto rit = relationsInverse.find(relation);
        if (rit != relationsInverse.end())
        {
            EntitiesSet const mapped = mappedBy(relation, srcEntity);
            for (ObjectId const&  dstEntity : mapped)
            {
                auto sit = rit->second.find(dstEntity);
                if (sit != rit->second.end())
                {
                    sit->second.erase(srcEntity);
                    if (sit->second.empty())
                    {
                        rit->second.erase(sit);
                        if (rit->second.empty())
                            relationsInverse.erase(rit);
                    }
                }
            }
        }
    }
    auto rit = relationsForward.find(relation);
    if (rit != relationsForward.end())
    {
        rit->second.erase(srcEntity);
        if (rit->second.empty())
            relationsForward.erase(rit);
    }
}


void WorldState::eraseRelation(Relation const& relation)
{
    fromRelationToFeatures.erase(relation);
    fromRelationToLabels.erase(relation);
    relationsForward.erase(relation);
    relationsInverse.erase(relation);
}


bool WorldState::hasRelation(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity) const
{
    return mappedBy(relation, srcEntity).count(dstEntity) != 0UL;
}


WorldState::EntitiesSet const& WorldState::mappedBy(Relation const& relation, ObjectId const& srcEntity) const
{
    auto rit = relationsForward.find(relation);
    if (rit == relationsForward.end())
        return emptyEntities;
    auto sit = rit->second.find(srcEntity);
    return sit == rit->second.end() ? emptyEntities : sit->second;
}


WorldState::EntitiesSet const& WorldState::mappedTo(Relation const& relation, ObjectId const& dstEntity) const
{
    auto rit = relationsInverse.find(relation);
    if (rit == relationsInverse.end())
        return emptyEntities;
    auto sit = rit->second.find(dstEntity);
    return sit == rit->second.end() ? emptyEntities : sit->second;
}


void WorldState::insertRelationFeature(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, Feature const& feature)
{
    insertRelation(relation, srcEntity, dstEntity);
    fromRelationToFeatures[relation][{srcEntity, dstEntity}].insert(feature);
}


void WorldState::eraseRelationFeature(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, Feature const& feature)
{
    auto rit = fromRelationToFeatures.find(relation);
    if (rit != fromRelationToFeatures.end())
    {
        auto fit = rit->second.find({srcEntity, dstEntity});
        if (fit != rit->second.end())
        {
            fit->second.erase(feature);
            if (fit->second.empty())
            {
                rit->second.erase(fit);
                if (rit->second.empty())
                    fromRelationToFeatures.erase(relation);
            }
        }
    }
}


void WorldState::eraseRelationFeatures(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity)
{
    auto rit = fromRelationToFeatures.find(relation);
    if (rit != fromRelationToFeatures.end())
    {
        rit->second.erase({srcEntity, dstEntity});
        if (rit->second.empty())
            fromRelationToFeatures.erase(relation);
    }
}


void WorldState::eraseRelationFeatures(Relation const& relation, ObjectId const& srcEntity)
{
    auto rit = fromRelationToFeatures.find(relation);
    if (rit != fromRelationToFeatures.end())
    {
        std::vector<RelationFeaturesMap::mapped_type::iterator> toErase;
        for (auto it = rit->second.begin(); it != rit->second.end(); ++it)
            if (it->first.first == srcEntity)
                toErase.push_back(it);
        for (auto it : toErase)
            rit->second.erase(it);
        if (rit->second.empty())
            fromRelationToFeatures.erase(relation);
    }
}


bool WorldState::hasRelationFeature(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, Feature const& feature) const
{
    return relationFeatures(relation, srcEntity, dstEntity).count(feature) != 0UL;
}


WorldState::FeaturesSet const& WorldState::relationFeatures(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity) const
{
    auto rit = fromRelationToFeatures.find(relation);
    if (rit != fromRelationToFeatures.end())
    {
        auto fit = rit->second.find({srcEntity, dstEntity});
        if (fit != rit->second.end())
            return fit->second;
    }
    return emptyFeatures;
}


void WorldState::insertRelationLabel(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, ObjectId const& label)
{
    insertRelation(relation, srcEntity, dstEntity);
    fromRelationToLabels[relation][{srcEntity, dstEntity}].insert(label);
}


void WorldState::eraseRelationLabel(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, ObjectId const& label)
{
    auto rit = fromRelationToLabels.find(relation);
    if (rit != fromRelationToLabels.end())
    {
        auto fit = rit->second.find({srcEntity, dstEntity});
        if (fit != rit->second.end())
        {
            fit->second.erase(label);
            if (fit->second.empty())
            {
                rit->second.erase(fit);
                if (rit->second.empty())
                    fromRelationToLabels.erase(relation);
            }
        }
    }
}


void WorldState::eraseRelationLabels(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity)
{
    auto rit = fromRelationToLabels.find(relation);
    if (rit != fromRelationToLabels.end())
    {
        rit->second.erase({srcEntity, dstEntity});
        if (rit->second.empty())
            fromRelationToLabels.erase(relation);
    }
}


void WorldState::eraseRelationLabels(Relation const& relation, ObjectId const& srcEntity)
{
    auto rit = fromRelationToLabels.find(relation);
    if (rit != fromRelationToLabels.end())
    {
        std::vector<RelationLabelsMap::mapped_type::iterator> toErase;
        for (auto it = rit->second.begin(); it != rit->second.end(); ++it)
            if (it->first.first == srcEntity)
                toErase.push_back(it);
        for (auto it : toErase)
            rit->second.erase(it);
        if (rit->second.empty())
            fromRelationToLabels.erase(relation);
    }
}


bool WorldState::hasRelationLabel(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity, ObjectId const& label) const
{
    return relationLabels(relation, srcEntity, dstEntity).count(label) != 0UL;
}


WorldState::EntitiesSet const& WorldState::relationLabels(Relation const& relation, ObjectId const& srcEntity, ObjectId const& dstEntity) const
{
    auto rit = fromRelationToLabels.find(relation);
    if (rit != fromRelationToLabels.end())
    {
        auto lit = rit->second.find({srcEntity, dstEntity});
        if (lit != rit->second.end())
            return lit->second;
    }
    return emptyEntities;
}


WorldState::EntitiesSet const  WorldState::emptyEntities;
WorldState::FeaturesSet const  WorldState::emptyFeatures;


std::unique_ptr<MessagePassing>  MessagePassing::clone() const
{
    std::unique_ptr<MessagePassing>  result = std::make_unique<MessagePassing>();
    *result = *this;
    return result;
}


bool MessagePassing::hasMessage(ObjectId const& receiver, Message const& message) const
{
    for (MessageInfo const&  msg : messages(receiver))
        if (msg.message == message)
            return true;
    return false;
}


bool MessagePassing::hasMessage(ObjectId const& receiver, ObjectId const& sender, Message const& message) const
{
    for (MessageInfo const&  msg : messages(receiver))
        if (msg.message == message && msg.sender == sender)
            return true;
    return false;
}


MessagePassing::MessageInfo const* MessagePassing::getMessageInfo(ObjectId const& receiver, Message const& message) const
{
    for (MessageInfo const&  msg : messages(receiver))
        if (msg.message == message)
            return &msg;
    return nullptr;
}


MessagePassing::MessageVector const& MessagePassing::messages(ObjectId const& receiver) const
{
    auto it = m_messages.find(receiver);
    return it == m_messages.end() ? emptyMessages : it->second;
}


void MessagePassing::sendMessage(ObjectId const& sender, ObjectId const& receiver, Message const& message, ObjectId const& location)
{
    m_pendingMessages[receiver].push_back(MessageInfo{ sender, message, location });
}


MessagePassing::MessageVector const& MessagePassing::history(ObjectId const& receiver) const
{
    auto it = m_history.find(receiver);
    return it == m_history.end() ? emptyMessages : it->second;
}


integer_32_bit  MessagePassing::indexOfReceivedMessage(ObjectId const& receiver, Message const& message) const
{
    integer_32_bit  idx = 0;
    for (MessageInfo const&  msg : history(receiver))
        if (msg.message == message)
            return idx;
        else
            ++idx;
    return -1;
}


integer_32_bit  MessagePassing::indexOfReceivedMessage(ObjectId const& receiver, ObjectId const& sender, Message const& message) const
{
    integer_32_bit  idx = 0;
    for (MessageInfo const&  msg : history(receiver))
        if (msg.sender == sender && msg.message == message)
            return idx;
        else
            ++idx;
    return -1;
}


bool MessagePassing::hasMessages() const
{
    return !m_messages.empty();
}


bool MessagePassing::hasMessages(ObjectId const& receiver) const
{
    return m_messages.find(receiver) != m_messages.end();
}


bool MessagePassing::hasPendingMessages() const
{
    return !m_pendingMessages.empty();
}


bool MessagePassing::hasPendingMessages(ObjectId const& receiver) const
{
    return m_pendingMessages.find(receiver) != m_pendingMessages.end();
}


void MessagePassing::deliverMessages(ObjectId const& receiver)
{
    auto const it = m_pendingMessages.find(receiver);
    if (it == m_pendingMessages.end())
        m_messages.erase(receiver);
    else
    {
        MessageVector& history = m_history[it->first];
        for (MessageInfo const&  msg : it->second)
            history.push_back(msg);

        m_messages[receiver].swap(it->second);
        m_pendingMessages.erase(it);
    }

}


MessagePassing::MessageVector const  MessagePassing::emptyMessages;


PlayerInteraction::TextLocalisation::TextLocalisation(std::ifstream& sstr)
    : objects()
    , features()
    , relations()
    , messages()
    , tokens()
    , actions()
    , descriptions()
{
    load(sstr);
}


void PlayerInteraction::TextLocalisation::clear()
{
    objects.clear();
    features.clear();
    relations.clear();
    messages.clear();
    tokens.clear();
    actions.clear();
    descriptions.clear();
}


void PlayerInteraction::TextLocalisation::load(std::ifstream& sstr)
{
    std::unordered_map<std::string, std::string>*  map = nullptr;
    std::string line;
    auto processLine = [this, &map](std::string& line) -> void {
        boost::trim(line);
        if (boost::starts_with(line, "//"))
        {
            line.clear();
            return;
        }
        std::size_t const idx = line.find(':');
        ASSUMPTION(line.empty() || idx != std::string::npos);
        std::string key = boost::trim_copy(line.substr(0UL, idx));
        std::string value = boost::trim_copy(boost::replace_all_copy(line.substr(idx + 1UL),"\\n","\n"));
        if (key.empty())
        {} // nothing to do, i.e. skip that line
        else if (value.empty())
        {
            if (key == "objects")
                map = &objects;
            else if (key == "features")
                map = &features;
            else if (key == "relations")
                map = &relations;
            else if (key == "messages")
                map = &messages;
            else if (key == "tokens")
                map = &tokens;
            else if (key == "actions")
                map = &actions;
            else if (key == "descriptions")
                map = &descriptions;
            else
                UNREACHABLE();
        }
        else
        {
            ASSUMPTION(map != nullptr);
            map->insert({ key, value });
        }
        line.clear();
    };
    for (auto it = std::istreambuf_iterator<char>(sstr), end = std::istreambuf_iterator<char>(); it != end; ++it)
    {
        if (*it == '\n')
            processLine(line);
        else if (*it != '\r')
            line.push_back(*it);
    }
    processLine(line);
}


PlayerInteraction::PlayerInteraction(
        std::shared_ptr<TextLocalisation const> const  localisation_,
        simulation_context_ptr const ctx_,
        osi::keyboard_props const& keyboard_,
        osi::mouse_props const& mouse_,
        osi::window_props const& window_,
        std::shared_ptr<gfx::font_mono_props const> const  font_,
        std::shared_ptr<gfx::viewport const> const  viewport_
        )
    : Serialisable()
    , actionSelectionProps(nullptr)
    , playerState(nullptr)
    , worldState(nullptr)
    , messages()
    , simulatorState(nullptr)
    , textLocalisation(localisation_)
    , ctx(ctx_)
    , keyboard(&keyboard_)
    , mouse(&mouse_)
    , window(&window_)
    , textBox(font_, viewport_)
    , inputText()
    , hasInvalidInput(false)
{
    ASSUMPTION(textLocalisation != nullptr);
}


void PlayerInteraction::processScene(float_32_bit const round_seconds)
{
    std::stringstream  sstr;

    processSceneToText(round_seconds, sstr);

    if (actionSelectionProps->actionInfos.empty())
        sstr << "\n" << textLocalisation->token("press_enter_to_continue");
    else
        sstr << "\n" << textLocalisation->token("choose_action_to_execute") << ": " << inputText << "_";

    textBox.set_text(sstr.str());
    textBox.update(round_seconds, *keyboard, *mouse);
}


bool PlayerInteraction::processMessage(MessagePassing::MessageInfo const&  message, float_32_bit const round_seconds)
{
    std::stringstream  sstr;

    processMessageToText(message, round_seconds, sstr);

    sstr << "\n" << textLocalisation->token("press_enter_to_continue");
    textBox.set_text(sstr.str());
    textBox.update(round_seconds, *keyboard, *mouse);
    return keyboard->keys_just_released().count(osi::KEY_RETURN()) != 0UL;
}


void PlayerInteraction::processMessageToText(
        MessagePassing::MessageInfo const&  message,
        float_32_bit const round_seconds,
        std::stringstream& sstr
        )
{
    if (message.location != com::age::invalid_object_id)
        sstr << textLocalisation->token("location") << ":\n  " << textLocalisation->object(message.location) << "\n";
    sstr << textLocalisation->token("sender") << ":\n  " << textLocalisation->object(message.sender) << "\n"
         << textLocalisation->token("message") << ":\n  " << textLocalisation->message(message.message) << "\n"
         ;
}


bool PlayerInteraction::processMessageInvalidActionIndex(std::string const& input, float_32_bit const round_seconds)
{
    std::stringstream  sstr;
    sstr << textLocalisation->token("inserted_invalid_action_index") << ": " << input << "\n\n"
         << textLocalisation->token("press_enter_to_continue");
    textBox.set_text(sstr.str());
    textBox.update(round_seconds, *keyboard, *mouse);
    if (keyboard->keys_just_released().count(osi::KEY_RETURN()) != 0UL)
    {
        inputText.clear();
        hasInvalidInput = false;
        return true;
    }
    return false;
}


std::size_t PlayerInteraction::readActionIndex(float_32_bit const round_seconds)
{
    ASSUMPTION(!hasInvalidInput);
    inputText += keyboard->typed_text();
    if (keyboard->keys_just_pressed().count(osi::KEY_BACKSPACE()) != 0UL && !inputText.empty())
        inputText.pop_back();
    if (keyboard->keys_just_released().count(osi::KEY_RETURN()) != 0UL)
    {
        if (actionSelectionProps->actionInfos.empty())
            return 0UL;
        if (!inputText.empty())
        {
            int const index = std::atoi(inputText.c_str());
            if (index >= 1 && (std::size_t)index <= actionSelectionProps->actionInfos.size())
            {
                return (std::size_t)(index - 1UL);
            }
            hasInvalidInput = true;
            return actionSelectionProps->actionInfos.size();
        }
    }
    return std::numeric_limits<std::size_t>::max();
}


bool PlayerInteraction::update(float_32_bit const round_seconds)
{
    if (!messages.empty())
    {
        if (processMessage(messages.back(), round_seconds))
            messages.pop_back();
    }
    else if (hasInvalidInput)
    {
        processMessageInvalidActionIndex(inputText, round_seconds);
    }
    else if (actionSelectionProps->chosenActionIndex == std::numeric_limits<std::size_t>::max())
    {
        std::size_t const  idx = readActionIndex(round_seconds);
        if (idx == std::numeric_limits<std::size_t>::max())
            processScene(round_seconds);
        else if (actionSelectionProps->actionInfos.empty() || idx < actionSelectionProps->actionInfos.size())
        {
            actionSelectionProps->chosenActionIndex = idx;
            return true;
        }
    }
    return false;
}


void PlayerInteraction::onNextPlayerActivated(
        com::age::ActorState const* playerState_,
        com::age::WorldState const* worldState_,
        MessagePassing::MessageVector const& messages_,
        com::age::SimulatorState const* simulatorState_
        )
{
    playerState = playerState_;
    worldState = worldState_;

    messages = messages_;
    std::reverse(messages.begin(), messages.end());

    simulatorState = simulatorState_;

    onNextPlayerActivated();
}


void PlayerInteraction::onPlayerLeft()
{
    actionSelectionProps = nullptr;

    inputText.clear();
    hasInvalidInput = false;
    textBox.set_text("");
    textBox.update(0.0f, *keyboard, *mouse);
}


void PlayerInteraction::render(gfx::draw_state& dstate) const
{
    textBox.render(dstate);
}


ActionHistory::Record ActionHistory::Record::make(ObjectId const actor_, PlayerAction const& action_, ActionContext const& context_)
{
    return make(actor_, typeid(action_).name(), context_);
}


ActionHistory::Record ActionHistory::Record::make(ObjectId const actor_, RobotAction const& action_, ActionContext const& context_)
{
    return make(actor_, typeid(action_).name(), context_);
}


ActionHistory::Record ActionHistory::Record::make(ObjectId const actor_, std::string const& action_, ActionContext const& context_)
{
    std::vector<ActionContext::AttributeNameAndValue> ctx;
    context_.dump(ctx);
    return { actor_, action_, ctx };
}


bool ActionHistory::Record::operator==(Record const& other) const
{
    if (actor != other.actor || action != other.action || context.size() != other.context.size())
        return false;
    for (std::size_t i = 0UL; i != context.size(); ++i)
        if (context.at(i) != other.context.at(i))
            return false;
    return true;
}


void ActionHistory::addAction(Record const& r)
{
    ASSUMPTION(isCurrentInPresent());
    data.push_back(r);
    current = data.size();
}


void ActionHistory::moveCurrentToNextRecord()
{
    ASSUMPTION(!isCurrentInPresent());
    ++current;
}


std::string ActionHistory::save(boost::property_tree::ptree& output) const
{

    boost::property_tree::ptree dat;
    for (Record const& r : data)
    {
        boost::property_tree::ptree rec;
        rec.put("actor", r.actor);
        rec.put("action", r.action);
        boost::property_tree::ptree ctx;
        for (ActionContext::AttributeNameAndValue const& c : r.context)
            ctx.put(c.first, c.second);
        rec.put_child("context", ctx);
        dat.push_back({ "", rec });
    }
    output.put_child("data", dat);
    return ""; // no error
}


std::string ActionHistory::load(boost::property_tree::ptree const& input)
{
    clear();
    boost::property_tree::ptree const& dat = input.get_child("data");
    for (auto it = dat.begin(); it != dat.end(); ++it)
    {
        boost::property_tree::ptree const& rec = it->second;
        boost::property_tree::ptree const& ctx = rec.get_child("context");

        Record r { rec.get<std::string>("actor"), rec.get<std::string>("action"), {} };
        for (auto cit = ctx.begin(); cit != ctx.end(); ++cit)
            r.context.push_back({ cit->first, cit->second.get_value<std::string>() });
        data.push_back(r);
    }
    return ""; // no error
}


SimulatorState::SimulatorState()
    : Serialisable()
    , robots()
    , players()
    , order()
    , activeActorIterator(order.end())
{}


void SimulatorState::insertRobot(
        std::unique_ptr<ActorState> robotState,
        std::shared_ptr<RobotScore> const robotScore,
        unsigned int const maxSearchDepth
        )
{
    ObjectId const actorId = robotState->self_id();
    robots.insert({actorId, ActorRobot(std::move(robotState), robotScore, maxSearchDepth)});
    order.push_back(actorId);
    if (activeActorIterator == order.end())
        activeActorIterator = order.begin();
}


void SimulatorState::insertPlayer(
        std::unique_ptr<ActorState> playerState,
        std::shared_ptr<PlayerInteraction> const interaction)
{
    ObjectId const actorId = playerState->self_id();
    players.insert({actorId, ActorPlayer(std::move(playerState), interaction)});
    order.push_back(actorId);
    if (activeActorIterator == order.end())
        activeActorIterator = order.begin();
}


void SimulatorState::eraseRobot(ObjectId const& robot)
{
    if (robot == activeActor())
        activatePreviousActor();
    robots.erase(robot);
    order.erase(std::find(order.begin(), order.end(), robot));
}


void SimulatorState::erasePlayer(ObjectId const& player)
{
    if (player == activeActor())
        activatePreviousActor();
    players.erase(player);
    order.erase(std::find(order.begin(), order.end(), player));
}


void SimulatorState::insertRobotAction(ObjectId const& robot, std::shared_ptr<RobotAction> const action)
{
    robots.at(robot).actions.insert({ std::type_index(typeid(*action)), action });
}


void SimulatorState::insertPlayerAction(ObjectId const& player, std::shared_ptr<PlayerAction> const action)
{
    players.at(player).actions.insert({ std::type_index(typeid(*action)), action });
}


void SimulatorState::eraseRobotAction(ObjectId const& robot, std::type_info const& actionType)
{
    robots.at(robot).actions.erase(std::type_index(actionType));
}


void SimulatorState::erasePlayerAction(ObjectId const& player, std::type_info const& actionType)
{
    players.at(player).actions.erase(std::type_index(actionType));
}


void SimulatorState::setActiveActor(ObjectId const& actor)
{
    activeActorIterator = std::find(order.begin(), order.end(), actor);
}


ObjectId const& SimulatorState::activeActor() const
{
    return activeActorIterator == order.end() ? invalid_object_id : *activeActorIterator;
}


void SimulatorState::activateNextActor()
{
    if (order.empty())
    {
        activeActorIterator = order.end();
        return;
    }
    if (activeActorIterator == order.end())
    {
        activeActorIterator = order.begin();
        return;
    }
    ++activeActorIterator;
    if (activeActorIterator == order.end())
        activeActorIterator = order.begin();
}


bool SimulatorState::hasPlayerInteractionMessages() const
{
    for (auto const&  id_and_player : players)
        if (!id_and_player.second.interaction->getMessages().empty())
            return true;
    return false;
}


void SimulatorState::activatePreviousActor()
{
    if (order.empty())
    {
        activeActorIterator = order.end();
        return;
    }
    if (activeActorIterator == order.end())
    {
        activeActorIterator = std::next(order.begin(), order.size() - 1UL);
        return;
    }
    if (activeActorIterator == order.begin())
        activeActorIterator = std::next(order.begin(), order.size() - 1UL);
    else
        --activeActorIterator;
}


std::unique_ptr<SimulatorState> SimulatorState::clone() const
{
    std::unique_ptr<SimulatorState> result = std::make_unique<SimulatorState>();
    for (auto const& id_and_robot : robots)
        result->robots.insert({id_and_robot.first, id_and_robot.second.clone() });
    for (auto const& id_and_player : players)
        result->players.insert({id_and_player.first, id_and_player.second.clone() });
    result->order = order;
    result->setActiveActor(activeActor());
    return result;
}


SimulatorState::ActorRobot::ActorRobot(
        std::unique_ptr<ActorState> robotState_,
        std::shared_ptr<RobotScore> const robotScore_,
        unsigned int const maxSearchDepth_,
        std::unordered_map<std::type_index, std::shared_ptr<RobotAction> > const& actions_
        )
    : robotState(std::move(robotState_))
    , robotScore(robotScore_)
    , maxSearchDepth(maxSearchDepth_)
    , actions(actions_)
{}

SimulatorState::ActorPlayer::ActorPlayer(
        std::unique_ptr<ActorState> playerState_,
        std::shared_ptr<PlayerInteraction> const interaction_,
        std::unordered_map<std::type_index, std::shared_ptr<PlayerAction> > const& actions_
        )
    : playerState(std::move(playerState_))
    , interaction(interaction_)
    , actions(actions_)
{}


struct SearchNode
{
    std::shared_ptr<RobotAction const> action;
    std::unique_ptr<ActionContext const> context;
    std::unique_ptr<ActorState> robotState;
    std::unique_ptr<WorldState> worldState;
    std::unique_ptr<MessagePassing> messaging;
    std::unique_ptr<SimulatorState> simulatorState;
    float score;
    unsigned int depth;
};


struct SearchTree
{
    int insert(
            std::shared_ptr<RobotAction const> action,
            std::unique_ptr<ActionContext const> context,
            std::unique_ptr<ActorState> robotState,
            std::unique_ptr<WorldState> worldState,
            std::unique_ptr<MessagePassing> messaging,
            std::unique_ptr<SimulatorState> simulatorState,
            float const score,
            int const parent
            )
    {
        int const idx = (int)nodes.size();
        nodes.push_back(SearchNode{
                action,
                std::move(context),
                std::move(robotState),
                std::move(worldState),
                std::move(messaging),
                std::move(simulatorState),
                score,
                parent == -1 ? 0 : nodes.at(parent).depth + 1
                });
        parents.push_back(parent);
        leaves.erase(parent);
        leaves.insert(idx);
        return idx;
    }

    std::vector<SearchNode> nodes;
    std::vector<int> parents;
    std::unordered_set<int> leaves;
};



void nextRound(
        WorldState& worldState,
        MessagePassing& messaging,
        SimulatorState& simulatorState,
        ActionHistory& history,
        float_32_bit const  round_seconds
        )
{
    TMPROF_BLOCK();

    ObjectId const&  actor_id = simulatorState.activeActor();
    if (actor_id == invalid_object_id || (history.getCurrent() != nullptr && history.getCurrent()->actor != actor_id))
    {
        simulatorState.activateNextActor();
        return;
    }

    if (simulatorState.players.count(actor_id) != 0UL)
    {
        SimulatorState::ActorPlayer& player = simulatorState.players.at(actor_id);

        if (player.interaction->actionSelectionProps == nullptr)
        {
            messaging.deliverMessages(actor_id);

            player.interaction->actionSelectionProps = std::make_unique<PlayerInteraction::ActionSelectionProps>();
            for (auto const& type_and_action : player.actions)
                type_and_action.second->contexts(*player.playerState, worldState, messaging, simulatorState,
                        [&type_and_action, &player](std::unique_ptr<ActionContext> context) {
                            player.interaction->actionSelectionProps->actionInfos.push_back({type_and_action.second, std::move(context)});
                        });

            player.interaction->onNextPlayerActivated(
                    player.playerState.get(),
                    &worldState,
                    messaging.messages(player.playerState->self_id()),
                    &simulatorState
                    );
        }
        if (!history.isCurrentInPresent() || player.interaction->update(round_seconds))
        {
            if (!player.interaction->actionSelectionProps->actionInfos.empty())
            {
                std::pair<std::shared_ptr<PlayerAction const>, std::unique_ptr<ActionContext const> > const*  bestInfo = nullptr;
                if (history.isCurrentInPresent())
                {
                    bestInfo = &player.interaction->actionSelectionProps->actionInfos.at(player.interaction->actionSelectionProps->chosenActionIndex);
                    history.addAction(actor_id, *bestInfo->first, *bestInfo->second);
                }
                else
                    for (PlayerInteraction::ActionAndContext const& actionInfo : player.interaction->actionSelectionProps->actionInfos)
                        if (history.isCurrent(actor_id, *actionInfo.first, *actionInfo.second))
                        {
                            bestInfo = &actionInfo;
                            history.moveCurrentToNextRecord();
                            break;
                        }
                ASSUMPTION(bestInfo != nullptr);
                bestInfo->first->apply(*bestInfo->second, *player.playerState, worldState, messaging, simulatorState);
            }

            player.interaction->onPlayerLeft();

            simulatorState.activateNextActor();
        }
    }
    else
    {
        messaging.deliverMessages(actor_id);

        SimulatorState::ActorRobot& robot = simulatorState.robots.at(actor_id);

        SearchTree tree;
        tree.insert(nullptr, nullptr, robot.robotState->clone(), worldState.clone(), messaging.clone(), simulatorState.clone(), 0.0f, -1);
        std::deque<int> queue;
        queue.push_back(0);
        while (!queue.empty() && tree.nodes.at(queue.front()).depth < robot.maxSearchDepth)
        {
            int const node_idx = queue.front();
            queue.pop_front();
            SearchNode const& node = tree.nodes.at(node_idx);
            if (node.simulatorState->robots.count(actor_id) == 0UL)
                continue;
            if (node.simulatorState->activeActor() == invalid_object_id)
                continue;
            std::vector<std::pair<std::shared_ptr<RobotAction const>, std::unique_ptr<ActionContext const> > >  actionInfos;
            for (auto const& type_and_action : node.simulatorState->robots.at(actor_id).actions)
                type_and_action.second->contexts(*node.robotState,  *node.worldState, *node.messaging, *node.simulatorState,
                        [&type_and_action, &actionInfos](std::unique_ptr<ActionContext> context) {
                            actionInfos.push_back({type_and_action.second, std::move(context)});
                        });
            for (auto&  action_and_context : actionInfos)
            {
                std::unique_ptr<ActorState> robotState = node.robotState->clone();
                std::unique_ptr<MessagePassing> messaging = node.messaging->clone();
                std::unique_ptr<WorldState> worldState = node.worldState->clone();
                std::unique_ptr<SimulatorState> simulatorState = node.simulatorState->clone();
                action_and_context.first->apply(*action_and_context.second, *robotState, *worldState, *messaging, *simulatorState);
                float const score = robot.robotScore->compute(*robotState, *worldState, *messaging, *simulatorState);
                simulatorState->activateNextActor();
                int const new_node_idx = tree.insert(
                        action_and_context.first,
                        std::move(action_and_context.second),
                        std::move(robotState),
                        std::move(worldState),
                        std::move(messaging),
                        std::move(simulatorState),
                        score,
                        node_idx);
                queue.push_back(new_node_idx);
            }
        }
        if (tree.nodes.size() > 1UL)
        {
            int best_node_idx = -1;
            for (int idx : tree.leaves)
                if (best_node_idx == -1 || tree.nodes.at(best_node_idx).score < tree.nodes.at(idx).score)
                    best_node_idx = idx;
            INVARIANT(best_node_idx != -1 && tree.parents.at(best_node_idx) != -1);
            while (tree.parents.at(tree.parents.at(best_node_idx)) != -1)
                best_node_idx = tree.parents.at(best_node_idx);
            SearchNode const*  bestNode = nullptr;
            if (history.isCurrentInPresent())
            {
                bestNode = &tree.nodes.at(best_node_idx);
                history.addAction(actor_id, *bestNode->action, *bestNode->context);
            }
            else
                for (SearchNode const& node : tree.nodes)
                    if (node.depth == 1U && history.isCurrent(actor_id, *node.action, *node.context))
                    {
                        bestNode = &node;
                        history.moveCurrentToNextRecord();
                        break;
                    }
            ASSUMPTION(bestNode != nullptr);
            bestNode->action->apply(*bestNode->context, *robot.robotState, worldState, messaging, simulatorState);
        }

        simulatorState.activateNextActor();
    }
}


void renderRound(SimulatorState const& simulatorState, gfx::draw_state& dstate)
{
    TMPROF_BLOCK();

    ObjectId const& actor_id = simulatorState.activeActor();
    if (actor_id == invalid_object_id || simulatorState.players.count(actor_id) == 0UL)
        return;
    simulatorState.players.at(actor_id).interaction->render(dstate);
}


std::shared_ptr<PlayerInteraction const> getPlayerInteraction(SimulatorState const& simulatorState, ObjectId const  playerId)
{
    if (playerId == invalid_object_id || simulatorState.players.count(playerId) == 0UL)
        return nullptr;
    return simulatorState.players.at(playerId).interaction;
}


std::string save(
        WorldState const& worldState,
        MessagePassing const& messaging,
        SimulatorState const& simulatorState,
        boost::property_tree::ptree&  ptree
        )
{
    // TODO!
    return "";
}


std::string load(
        boost::property_tree::ptree const&  ptree,
        WorldState& worldState,
        MessagePassing& messaging,
        SimulatorState& simulatorState
        )
{
    // TODO!
    return "";
}


}}
