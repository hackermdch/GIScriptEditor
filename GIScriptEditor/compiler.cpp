module;
#include <GINodeGraph.h>
module compiler;

using namespace Editor::Tools;

struct EventParameter
{
	std::string id;
	Script::VarType type;
};

struct GenericPins
{
	std::unordered_map<unsigned, std::unordered_map<Script::VarType, unsigned>> in_pins;
	std::unordered_map<unsigned, std::unordered_map<Script::VarType, unsigned>> out_pins;
};

struct EventProto
{
	NodeId id;
	std::vector<EventParameter> parameters;
	std::shared_ptr<GenericPins> generic_pins;

	bool Contains(unsigned pin, bool out = false) const
	{
		if (!generic_pins) return false;
		return (out ? generic_pins->out_pins : generic_pins->in_pins).contains(pin);
	}

	INode& Create(IGraph& graph) const
	{
		auto& node = graph.AddNode(id);
		if (generic_pins)
		{
			for (unsigned i = 0; i < parameters.size(); i++)
			{
				if (auto pin = generic_pins->out_pins.find(i); pin != generic_pins->out_pins.end())
				{
					node.Set(i, pin->second.at(parameters[i].type), true);
				}
			}
		}
		return node;
	}
};

static class EventRegistry
{
	std::unordered_map<std::string, std::list<EventProto>> registries;

	void Register(const std::string& name, std::initializer_list<EventParameter> ps, NodeId id, std::shared_ptr<GenericPins> gps = nullptr)
	{
		registries[name].emplace_back(id, ps, std::move(gps));
	}

	static bool Find(const EventProto& proto, const Variable& v)
	{
		for (const auto& [id, type] : proto.parameters)
		{
			if (id == v.Id())
			{
				if (type == v.Type()) return true;
			}
		}
		return false;
	}

	static Script::VarType Int() { return { Script::VarType::Int, {} }; }
	static Script::VarType Float() { return { Script::VarType::Float, {} }; }
	static Script::VarType String() { return { Script::VarType::String, {} }; }
	static Script::VarType Bool() { return { Script::VarType::Bool, {} }; }
	static Script::VarType Entity() { return { Script::VarType::Entity, {} }; }
	static Script::VarType Vec() { return { Script::VarType::Vec, {} }; }
	static Script::VarType Guid() { return { Script::VarType::Guid, GuidEx::Entity }; }
	static Script::VarType Prefab() { return { Script::VarType::Guid, GuidEx::Prefab }; }
	static Script::VarType Cfg() { return { Script::VarType::Guid, GuidEx::Configuration }; }
	static Script::VarType Faction() { return { Script::VarType::Guid, GuidEx::Faction }; }
	static Script::VarType List(Script::VarType element) { return { Script::VarType::List, element }; }
	static Script::VarType Map(MapEx ex) { return { Script::VarType::Map, ex }; }

	static EventParameter Int(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Int, {} } }; }
	static EventParameter Float(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Float, {} } }; }
	static EventParameter String(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::String, {} } }; }
	static EventParameter Bool(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Bool, {} } }; }
	static EventParameter Entity(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Entity, {} } }; }
	static EventParameter Vec(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Vec, {} } }; }
	static EventParameter Guid(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Guid, GuidEx::Entity } }; }
	static EventParameter Prefab(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Guid, GuidEx::Prefab } }; }
	static EventParameter Cfg(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Guid, GuidEx::Configuration } }; }
	static EventParameter Faction(std::string id) { return { std::move(id), Script::VarType{ Script::VarType::Guid, GuidEx::Faction } }; }
	static EventParameter List(std::string id, Script::VarType element) { return { std::move(id), Script::VarType{ Script::VarType::List, element } }; }
	static EventParameter Map(std::string id, MapEx ex) { return { std::move(id), Script::VarType{ Script::VarType::Map, ex } }; }
public:
	EventRegistry()
	{
		using enum NodeId;
		Register("OnEntityCreated", { Entity("sourceEntity"),Guid("sourceGuid") }, WhenEntityIsCreated);
		Register("OnEntityRemovedDestroyed", { Guid("source") }, WhenEntityIsRemovedDestroyed);
		Register("OnPresetStatusChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a"),Int("b"),Int("c") }, WhenPresetStatusChanges);
		Register("OnTimerIsTriggered", { Entity("sourceEntity"),Guid("sourceGuid"),String("a"),Int("b"),Int("c"),Guid("d") }, WhenTimerIsTriggered);
		Register("OnBasicMotionDeviceStops", { Entity("sourceEntity"),Guid("sourceGuid"),String("a") }, WhenBasicMotionDeviceStops);
		Register("OnExitingCollisionTrigger", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Guid("b"),Int("c") }, WhenExitingCollisionTrigger);
		Register("OnEnteringCollisionTrigger", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Guid("b"),Int("c") }, WhenEnteringCollisionTrigger);
		Register("OnPathReachesWaypoint", { Entity("sourceEntity"),Guid("sourceGuid"),String("a"),Int("b"),Int("c") }, WhenPathReachesWaypoint);
		Register("OnEntityFactionChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Faction("a"),Faction("b") }, WhenEntityFactionChanges);
		Register("OnOnHitDetectionIsTriggered", { Entity("sourceEntity"),Guid("sourceGuid"),Bool("a"),Entity("b"),Vec("c") }, WhenOnHitDetectionIsTriggered);
		Register("OnCharacterRevives", { Entity("sourceEntity") }, WhenCharacterRevives);
		Register("OnAllPlayersCharactersAreDown", { Entity("sourceEntity") }, WhenAllPlayersCharactersAreDown);
		Register("OnPlayerIsAbnormallyDownedandRevives", { Entity("sourceEntity") }, WhenPlayerIsAbnormallyDownedandRevives);
		Register("OnAllPlayersCharactersAreRevived", { Entity("sourceEntity") }, WhenAllPlayersCharactersAreRevived);
		Register("OnPlayerTeleportCompletes", { Entity("sourceEntity"),Guid("sourceGuid") }, WhenPlayerTeleportCompletes);
		Register("OnUnitStatusChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Entity("b"),Bool("c"),Float("d"),Int("e"),Int("f"),Int("g") }, WhenUnitStatusChanges);
		Register("OnTabIsSelected", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a"),Entity("b"),Guid("c") }, WhenTabIsSelected);
		Register("OnGlobalTimerIsTriggered", { Entity("sourceEntity"),Guid("sourceGuid"),String("a") }, WhenGlobalTimerIsTriggered);
		Register("OnUIControlGroupIsTriggered", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a"),Int("b") }, WhenUIControlGroupIsTriggered);
		Register("OnCreationEntersCombat", { Entity("sourceEntity"),Guid("sourceGuid") }, WhenCreationEntersCombat);
		Register("OnCreationLeavesCombat", { Entity("sourceEntity"),Guid("sourceGuid") }, WhenCreationLeavesCombat);
		Register("OnPlayerClassChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Cfg("b") }, WhenPlayerClassChanges);
		Register("OnPlayerClassLevelChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a"),Int("b") }, WhenPlayerClassLevelChanges);
		Register("OnSkillNodeIsCalled", { Entity("sourceEntity"),Guid("sourceGuid"),String("a"),String("b"),String("c") }, WhenSkillNodeIsCalled);
		Register("OnHPIsRecovered", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Float("b"),List("c", String()) }, WhenHPIsRecovered);
		Register("OnInitiatingHPRecovery", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Float("b"),List("c", String()) }, WhenInitiatingHPRecovery);
		Register("OnAggroTargetChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Entity("b") }, WhenAggroTargetChanges);
		Register("OnSelfEntersCombat", { Entity("sourceEntity"),Guid("sourceGuid") }, WhenSelfEntersCombat);
		Register("OnSelfLeavesCombat", { Entity("sourceEntity"),Guid("sourceGuid") }, WhenSelfLeavesCombat);
		Register("OnCreationReachesPatrolWaypoint", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a"),Int("b"),Int("c"),Int("d") }, WhenCreationReachesPatrolWaypoint);
		Register("OnShieldIsAttacked", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Guid("b"),Cfg("c"),Int("d"),Int("e"),Float("f"),Float("g") }, WhenShieldIsAttacked);
		Register("OnTextBubbleIsCompleted", { Entity("sourceEntity"),Entity("sourceGuid"),Cfg("a"),Int("b") }, WhenTextBubbleIsCompleted);
		Register("OnEquipmentAffixValueChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a"),Int("b"),Float("c"),Float("d") }, WhenEquipmentAffixValueChanges);
		Register("OnItemIsAddedtoInventory", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Int("b") }, WhenItemIsAddedtoInventory);
		Register("OnItemIsLostFromInventory", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Int("b") }, WhenItemIsLostFromInventory);
		Register("OntheQuantityofInventoryItemChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Int("b"),Int("c") }, WhentheQuantityofInventoryItemChanges);
		Register("OntheQuantityofInventoryCurrencyChanges", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Int("b") }, WhentheQuantityofInventoryCurrencyChanges);
		Register("OnEquipmentIsInitialized", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a") }, WhenEquipmentIsInitialized);
		Register("OnEquipmentIsEquipped", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a") }, WhenEquipmentIsEquipped);
		Register("OnEquipmentIsUnequipped", { Entity("sourceEntity"),Guid("sourceGuid"),Int("a") }, WhenEquipmentIsUnequipped);
		Register("OnCustomShopItemIsSold", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Int("b"),Int("c"),Int("d") }, WhenCustomShopItemIsSold);
		Register("OnSellingInventoryItemsintheShop", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a"),Int("b"),Cfg("c"),Int("d") }, WhenSellingInventoryItemsintheShop);
		Register("OnItemsintheInventoryAreUsed", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Int("b") }, WhenItemsintheInventoryAreUsed);
		Register("OnPlayerClassIsRemoved", { Entity("sourceEntity"),Guid("sourceGuid"),Cfg("a"),Cfg("b") }, WhenPlayerClassIsRemoved);
		Register("OnEnteringanInterruptibleState", { Entity("sourceEntity"),Guid("sourceGuid"),Entity("a") }, WhenEnteringanInterruptibleState);
		{
			auto generic = std::make_shared<GenericPins>();
			auto& [in_pins, out_pins] = *generic;
			out_pins[3] = out_pins[4] =
			{
				{ Int(), 0 },
				{ String(), 1 },
				{ Entity(), 2 },
				{ Guid(), 3 },
				{ Float(), 4 },
				{ Vec(), 5 },
				{ Bool(), 6 },
				{ Cfg(), 14 },
				{ Prefab(), 15 },
				{ Faction(), 18 },
				{ List(Int()), 7 },
				{ List(String()), 8 },
				{ List(Entity()), 9 },
				{ List(Guid()), 10 },
				{ List(Float()), 11 },
				{ List(Vec()), 12 },
				{ List(Bool()), 13 },
				{ List(Cfg()), 16 },
				{ List(Prefab()), 17 },
				{ List(Faction()), 19 }
			};
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Int("before"),Int("after") }, WhenCustomVariableChangesInt, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),String("before"),String("after") }, WhenCustomVariableChangesStr, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Entity("before"),Entity("after") }, WhenCustomVariableChangesEntity, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Guid("before"),Guid("after") }, WhenCustomVariableChangesGUID, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Float("before"),Float("after") }, WhenCustomVariableChangesFloat, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Vec("before"),Vec("after") }, WhenCustomVariableChangesVec, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Bool("before"),Bool("after") }, WhenCustomVariableChangesBool, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Cfg("before"),Cfg("after") }, WhenCustomVariableChangesConfig, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Prefab("before"),Prefab("after") }, WhenCustomVariableChangesPrefab, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),Faction("before"),Faction("after") }, WhenCustomVariableChangesFaction, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Int()), List("after", Int()) }, WhenCustomVariableChangesListInt, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", String()), List("after", String()) }, WhenCustomVariableChangesListStr, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Entity()), List("after", Entity()) }, WhenCustomVariableChangesListEntity, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Guid()), List("after", Guid()) }, WhenCustomVariableChangesListGUID, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Float()), List("after", Float()) }, WhenCustomVariableChangesListFloat, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Vec()), List("after", Vec()) }, WhenCustomVariableChangesListVec, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Bool()), List("after", Bool()) }, WhenCustomVariableChangesListBool, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Cfg()), List("after", Cfg()) }, WhenCustomVariableChangesListConfig, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Prefab()), List("after", Prefab()) }, WhenCustomVariableChangesListPrefab, generic);
			Register("OnCustomVariableChanges", { Entity("sourceEntity"),Guid("sourceGuid"),String("name"),List("before", Faction()), List("after", Faction()) }, WhenCustomVariableChangesListFaction, generic);
		}
	}

	const EventProto& Lookup(const std::string& name, const std::vector<Variable>& parameters)
	{
		auto it = registries.find(name);
		if (it == registries.end()) throw std::runtime_error("Unknown event: " + name);
		auto& overloads = it->second;
		if (overloads.size() == 1 && parameters.empty()) return overloads.front();
		for (auto& proto : overloads)
		{
			for (auto& p : parameters)
			{
				if (!Find(proto, p)) goto next;
			}
			return proto;
		next:
			(void)0;
		}
		throw std::runtime_error("No matching overload for event: " + name);
	}
} EventRegistry;

struct FunctionProto
{
	std::optional<Script::VarType> ret;
	std::vector<Script::VarType> parameters;
	std::shared_ptr<GenericPins> generic_pins;
	NodeId id;
	bool pure;

	FunctionProto(std::optional<Script::VarType> ret, std::initializer_list<Script::VarType> ps, NodeId id, bool pure, std::shared_ptr<GenericPins> gps) : ret(std::move(ret)), parameters(ps), generic_pins(std::move(gps)), id(id), pure(pure)
	{
	}

	bool Contains(unsigned pin, bool out = false) const
	{
		if (!generic_pins) return false;
		return (out ? generic_pins->out_pins : generic_pins->in_pins).contains(pin);
	}

	std::unique_ptr<INode> Create(IGraph& graph) const
	{
		auto node = graph.CreateNode(id);
		if (generic_pins)
		{
			for (unsigned i = 0; i < parameters.size(); i++)
			{
				if (auto pin = generic_pins->in_pins.find(i); pin != generic_pins->in_pins.end())
				{
					node->Set(i, pin->second.at(parameters[i]), false);
				}
			}
			if (ret.has_value())
			{
				if (ret->type == Script::VarType::Tuple)
				{
					auto& types = std::any_cast<const std::vector<Script::VarType>&>(ret->extra);
					for (unsigned i = 0; i < types.size(); i++)
					{
						if (auto pin = generic_pins->out_pins.find(i); pin != generic_pins->out_pins.end())
						{
							node->Set(i, pin->second.at(types[i]), true);
						}
					}
				}
				else if (auto pin = generic_pins->out_pins.find(0); pin != generic_pins->out_pins.end()) node->Set(0, pin->second.at(*ret), true);
			}
		}
		return node;
	}
};

static class FunctionRegistry
{
	std::unordered_map<std::string, std::list<FunctionProto>> registries;

	void Register(const std::string& name, std::optional<Script::VarType> ret, std::initializer_list<Script::VarType> ps, NodeId id, bool pure = false, std::shared_ptr<GenericPins> gps = nullptr)
	{
		registries[name].emplace_back(std::move(ret), ps, id, pure, gps);
	}

	static Script::VarType Int() { return { Script::VarType::Int, {} }; }
	static Script::VarType Float() { return { Script::VarType::Float, {} }; }
	static Script::VarType String() { return { Script::VarType::String, {} }; }
	static Script::VarType Bool() { return { Script::VarType::Bool, {} }; }
	static Script::VarType Entity() { return { Script::VarType::Entity, {} }; }
	static Script::VarType Vec() { return { Script::VarType::Vec, {} }; }
	static Script::VarType Guid() { return { Script::VarType::Guid, GuidEx::Entity }; }
	static Script::VarType Prefab() { return { Script::VarType::Guid, GuidEx::Prefab }; }
	static Script::VarType Cfg() { return { Script::VarType::Guid, GuidEx::Configuration }; }
	static Script::VarType Faction() { return { Script::VarType::Guid, GuidEx::Faction }; }
	static Script::VarType List(Script::VarType element) { return { Script::VarType::List, element }; }
	static Script::VarType Map(MapEx ex) { return { Script::VarType::Map, ex }; }
public:
	class Ref
	{
		friend FunctionRegistry;
		decltype(registries)::iterator it;
		Ref(decltype(it) it) :it(std::move(it)) {}
	};

	FunctionRegistry()
	{
		using enum NodeId;
		Register("print", {}, { String() }, PrintString);
		Register("ForwardEvent", {}, { Entity() }, ForwardingEvent);
		Register("GetRandomFloatingPointNumber", { Float() }, { Float(),Float() }, GetRandomFloatingPointNumber, true);
		Register("WeightedRandom", { Int() }, { List(Int()) }, WeightedRandom, true);
		Register("SetPresetStatus", {}, { Entity(),Int(),Int() }, SetPresetStatus);
		Register("GetPresetStatus", { Int() }, { Entity(),Int() }, GetPresetStatus, true);
		Register("DestroyEntity", {}, { Entity() }, DestroyEntity);
		Register("CreateEntity", {}, { Guid(),List(Int()) }, CreateEntity);
		Register("GetSelfEntity", { Entity() }, {}, GetSelfEntity, true);
		Register("QueryEntitybyGUID", { Entity() }, { Guid() }, QueryEntitybyGUID, true);
		Register("QueryGUIDbyEntity", { Guid() }, { Entity() }, QueryGUIDbyEntity, true);
		Register("SettleStage", {}, { Bool() }, SettleStage);
		Register("StartTimer", {}, { Entity(),String(),Bool(),List(Float()) }, StartTimer);
		Register("PauseTimer", {}, { Entity(),String() }, PauseTimer);
		Register("ResumeTimer", {}, { Entity(),String() }, ResumeTimer);
		Register("StopTimer", {}, { Entity(),String() }, StopTimer);
		Register("AddUniformBasicLinearMotionDevice", {}, { Entity(),String(),Float(),Vec() }, AddUniformBasicLinearMotionDevice);
		Register("AddUniformBasicRotationBasedMotionDevice", {}, { Entity(),String(),Float(),Float(),Vec() }, AddUniformBasicRotationBasedMotionDevice);
		Register("StopandDeleteBasicMotionDevice", {}, { Entity(),String(),Bool() }, StopandDeleteBasicMotionDevice);
		Register("PauseBasicMotionDevice", {}, { Entity(),String() }, PauseBasicMotionDevice);
		Register("RecoverBasicMotionDevice", {}, { Entity(),String() }, RecoverBasicMotionDevice);
		Register("ActivateDisableCollisionTrigger", {}, { Entity(),Int(),Bool() }, ActivateDisableCollisionTrigger);
		Register("PlayTimedEffects", {}, { Cfg(),Entity(),String(),Bool(),Bool(),Vec(),Vec(),Float(),Bool() }, PlayTimedEffects);
		Register("MountLoopingSpecialEffect", { Int() }, { Cfg(),Entity(),String(),Bool(),Bool(),Vec(),Vec(),Float(),Bool() }, MountLoopingSpecialEffect);
		Register("ClearLoopingSpecialEffect", {}, { Int(),Entity() }, ClearLoopingSpecialEffect);
		Register("ActivateDisableEntityDeploymentGroup", {}, { Int(),Bool() }, ActivateDisableEntityDeploymentGroup);
		Register("GetCurrentlyActiveEntityDeploymentGroups", { List(Int()) }, {}, GetCurrentlyActiveEntityDeploymentGroups, true);
		Register("ForwardingEvent", {}, { Entity() }, ForwardingEvent);
		Register("Pi", { Float() }, { Float() }, Pi, true);
		Register("ModuloOperation", { Int() }, { Int(),Int() }, ModuloOperation, true);
		Register("LogarithmOperation", { Float() }, { Float(),Float() }, LogarithmOperation, true);
		Register("ArithmeticSquareRootOperation", { Float() }, { Float() }, ArithmeticSquareRootOperation, true);
		Register("RoundtoIntegerOperation", { Int() }, { Float() }, RoundtoIntegerOperation, true);
		Register("Create3DVector", { Vec() }, { Float(),Float(),Float() }, Create3DVector, true);
		Register("LogicalANDOperation", { Bool() }, { Bool(),Bool() }, LogicalANDOperation, true);
		Register("LogicalOROperation", { Bool() }, { Bool(),Bool() }, LogicalOROperation, true);
		Register("LogicalXOROperation", { Bool() }, { Bool(),Bool() }, LogicalXOROperation, true);
		Register("LogicalNOTOperation", { Bool() }, { Bool(),Bool() }, LogicalNOTOperation, true);
		Register("ActivateDisableNativeCollision", {}, { Entity(),Bool() }, ActivateDisableNativeCollision);
		Register("ActivateDisableNativeCollisionClimbability", {}, { Entity(),Bool() }, ActivateDisableNativeCollisionClimbability);
		Register("ActivateDisableExtraCollision", {}, { Entity(),Int(),Bool() }, ActivateDisableExtraCollision);
		Register("ActivateDisableExtraCollisionClimbability", {}, { Entity(),Int(),Bool() }, ActivateDisableExtraCollisionClimbability);
		Register("DistanceBetweenTwoCoordinatePoints", { Float() }, { Vec(),Vec() }, DistanceBetweenTwoCoordinatePoints, true);
		Register("SwitchFollowMotionDeviceTargetbyGUID", {}, { Entity(),Guid(),String(),Vec(),Vec() }, SwitchFollowMotionDeviceTargetbyGUID);
		Register("GetListofPlayerEntitiesontheField", { List(Entity()) }, {}, GetListofPlayerEntitiesontheField, true);
		Register("QueryEntityFaction", { Faction() }, { Entity() }, QueryEntityFaction, true);
		Register("ModifyEntityFaction", {}, { Entity(),Faction() }, ModifyEntityFaction);
		Register("CreatePrefab", { Entity() }, { Prefab(),Vec(),Vec(),Entity(),Entity(),Bool(),Int(),List(Int()) }, CreatePrefab);
		Register("CreateProjectile", { Entity() }, { Prefab(),Vec(),Vec(),Entity(),Entity(),Bool(),Int(),List(Int()) }, CreateProjectile);
		Register("GetRandomInteger", { Int() }, { Int(),Int() }, GetRandomInteger, true);
		Register("GetAllCharacterEntitiesofSpecifiedPlayer", { List(Entity()) }, { Entity() }, GetAllCharacterEntitiesofSpecifiedPlayer, true);
		Register("GetPlayerEntitytoWhichtheCharacterBelongs", { Entity() }, { Entity() }, GetPlayerEntitytoWhichtheCharacterBelongs, true);
		Register("GetEntityType", {}, { Entity() }, GetEntityType, true);
		Register("SwitchMainCameraTemplate", {}, { List(Entity()),String() }, SwitchMainCameraTemplate);
		Register("ActivateEntityCamera", {}, { List(Entity()),Entity() }, ActivateEntityCamera, true);
		Register("DisableEntityCamera", {}, { List(Entity()) }, DisableEntityCamera, true);
		Register("ActivateFocusCamera", {}, { List(Entity()),Entity() }, ActivateFocusCamera, true);
		Register("DisableFocusCamera", {}, { List(Entity()) }, DisableFocusCamera, true);
		Register("ActivateScreenShake", {}, { List(Entity()),Float(),Float(),Float() }, ActivateScreenShake, true);
		Register("ActivateBasicMotionDevice", {}, { Entity(),String() }, ActivateBasicMotionDevice);
		Register("GetPresetPointListbyUnitTag", { List(Int()) }, { Int() }, GetPresetPointListbyUnitTag, true);
		Register("ActivateRevivePoint", {}, { Entity(),Int() }, ActivateRevivePoint);
		Register("DeactivateRevivePoint", {}, { Entity(),Int() }, DeactivateRevivePoint);
		Register("AllowForbidPlayertoRevive", {}, { Entity(),Bool() }, AllowForbidPlayertoRevive);
		Register("GetPlayerRemainingRevives", { Int() }, { Entity() }, GetPlayerRemainingRevives, true);
		Register("SetPlayerRemainingRevives", {}, { Entity(),Int() }, SetPlayerRemainingRevives);
		Register("GetPlayerReviveTime", { Int() }, { Entity() }, GetPlayerReviveTime, true);
		Register("SetPlayerReviveTime", {}, { Entity(),Int() }, SetPlayerReviveTime);
		Register("ReviveCharacter", {}, { Entity() }, ReviveCharacter);
		Register("DefeatAllPlayersCharacters", {}, { Entity() }, DefeatAllPlayersCharacters);
		Register("ReviveAllPlayersCharacters", {}, { Entity(),Bool() }, ReviveAllPlayersCharacters);
		Register("QueryIfAllPlayerCharactersAreDown", { Bool() }, { Entity() }, QueryIfAllPlayerCharactersAreDown, true);
		Register("TeleportPlayer", {}, { Entity(),Vec(),Vec() }, TeleportPlayer);
		Register("QueryGameTimeElapsed", { Int() }, {}, QueryGameTimeElapsed, true);
		Register("SineFunction", { Float() }, { Float() }, SineFunction, true);
		Register("CosineFunction", { Float() }, { Float() }, CosineFunction, true);
		Register("TangentFunction", { Float() }, { Float() }, TangentFunction, true);
		Register("ArcsineFunction", { Float() }, { Float() }, ArcsineFunction, true);
		Register("ArccosineFunction", { Float() }, { Float() }, ArccosineFunction, true);
		Register("ArctangentFunction", { Float() }, { Float() }, ArctangentFunction, true);
		Register("ModifyingCharacterDisruptorDevice", {}, { Entity(),Int() }, ModifyingCharacterDisruptorDevice);
		Register("InitiateAttack", {}, { Entity(),Float(),Float(),Vec(),Vec(),String(),Bool(),Entity() }, InitiateAttack);
		Register("ActivateDisableTab", {}, { Entity(),Int(),Bool() }, ActivateDisableTab);
		Register("ActivateDisableModelDisplay", {}, { Entity(),Bool() }, ActivateDisableModelDisplay);
		Register("PauseGlobalTimer", {}, { Entity(),String() }, PauseGlobalTimer);
		Register("GetCurrentGlobalTimerTime", { Float() }, { Entity(),String() }, GetCurrentGlobalTimerTime, true);
		Register("StartGlobalTimer", {}, { Entity(),String() }, StartGlobalTimer);
		Register("RecoverGlobalTimer", {}, { Entity(),String() }, RecoverGlobalTimer);
		Register("StopGlobalTimer", {}, { Entity(),String() }, StopGlobalTimer);
		Register("ModifyGlobalTimer", {}, { Entity(),String(),Float() }, ModifyGlobalTimer);
		Register("GetPlayersCurrentUILayout", { Int() }, { Entity() }, GetPlayersCurrentUILayout, true);
		Register("GetAllEntitiesontheField", { List(Entity()) }, {}, GetAllEntitiesontheField, true);
		Register("GetSpecifiedTypeofEntitiesontheField", { List(Entity()) }, {}, GetSpecifiedTypeofEntitiesontheField, true);
		Register("GetEntitiesWithSpecifiedPrefabontheField", { List(Entity()) }, { Prefab() }, GetEntitiesWithSpecifiedPrefabontheField, true);
		Register("RadianstoDegrees", { Float() }, { Float() }, RadianstoDegrees, true);
		Register("DegreestoRadians", { Float() }, { Float() }, DegreestoRadians, true);
		Register("RemoveEntity", {}, { Entity() }, RemoveEntity);
		Register("GetCreationsCurrentTarget", { Entity() }, { Entity() }, GetCreationsCurrentTarget, true);
		Register("GetEntityListbySpecifiedType", { List(Entity()) }, { List(Entity()) }, GetEntityListbySpecifiedType, true);
		Register("GetEntityListbySpecifiedPrefab", { List(Entity()) }, { List(Entity()),Prefab() }, GetEntityListbySpecifiedPrefab, true);
		Register("GetEntityListbySpecifiedFaction", { List(Entity()) }, { List(Entity()),Faction() }, GetEntityListbySpecifiedFaction, true);
		Register("GetEntityListbySpecifiedRange", { List(Entity()) }, { List(Entity()),Vec(),Float() }, GetEntityListbySpecifiedRange, true);
		Register("SwitchCurrentInterfaceLayout", {}, { Entity(),Int() }, SwitchCurrentInterfaceLayout);
		Register("ActivateUIControlGroupinControlGroupLibrary", {}, { Entity(),Int() }, ActivateUIControlGroupinControlGroupLibrary);
		Register("ModifyUIControlStatusWithintheInterfaceLayout", {}, { Entity(),Int() }, ModifyUIControlStatusWithintheInterfaceLayout);
		Register("QueryPlayerClass", { Cfg() }, { Entity() }, QueryPlayerClass, true);
		Register("QueryPlayerClassLevel", { Int() }, { Entity(),Cfg() }, QueryPlayerClassLevel, true);
		Register("ChangePlayerClass", {}, { Entity(),Cfg() }, ChangePlayerClass);
		Register("IncreasePlayersCurrentClassEXP", {}, { Entity(),Int() }, IncreasePlayersCurrentClassEXP);
		Register("ChangePlayersCurrentClassLevel", {}, { Entity(),Int() }, ChangePlayersCurrentClassLevel);
		Register("ModifySkillResourceAmount", {}, { Entity(),Cfg(),Float() }, ModifySkillResourceAmount);
		Register("SetSkillResourceAmount", {}, { Entity(),Cfg(),Float() }, SetSkillResourceAmount);
		Register("AddCharacterSkill", {}, { Entity(),Cfg() }, AddCharacterSkill);
		Register("DeleteCharacterSkillbyID", {}, { Entity(),Cfg() }, DeleteCharacterSkillbyID);
		Register("InitializeCharacterSkill", {}, { Entity() }, InitializeCharacterSkill);
		Register("QueryCharacterSkill", { Cfg() }, { Entity() }, QueryCharacterSkill, true);
		Register("DeleteCharacterSkillbySlot", {}, { Entity() }, DeleteCharacterSkillbySlot);
		Register("ClearSpecialEffectsBasedonSpecialEffectAssets", {}, { Entity(),Cfg() }, ClearSpecialEffectsBasedonSpecialEffectAssets);
		Register("QueryIfEntityIsontheField", { Bool() }, { Entity() }, QueryIfEntityIsontheField, true);
		Register("QueryIfEntityHasUnitStatus", { Bool() }, { Entity(),Cfg() }, QueryIfEntityHasUnitStatus, true);
		Register("GetEntityForwardVector", { Vec() }, { Entity() }, GetEntityForwardVector, true);
		Register("GetEntityRightVector", { Vec() }, { Entity() }, GetEntityRightVector, true);
		Register("GetEntityUpwardVector", { Vec() }, { Entity() }, GetEntityUpwardVector, true);
		Register("DirectionVectortoRotation", { Vec() }, { Vec(),Vec() }, DirectionVectortoRotation, true);
		Register("AddTargetOrientedRotationBasedMotionDevice", {}, { Entity(),String(),Float(),Vec() }, AddTargetOrientedRotationBasedMotionDevice);
		Register("RemoveInterfaceControlGroupFromControlGroupLibrary", {}, { Entity(),Int() }, RemoveInterfaceControlGroupFromControlGroupLibrary);
		Register("RecoverHP", {}, { Entity(),Float(),String(),Bool(),Entity() }, RecoverHP);
		Register("AddUnitTagtoEntity", {}, { Entity(),Int() }, AddUnitTagtoEntity);
		Register("RemoveUnitTagfromEntity", {}, { Entity(),Int() }, RemoveUnitTagfromEntity);
		Register("ClearUnitTagsfromEntity", {}, { Entity() }, ClearUnitTagsfromEntity);
		Register("GetEntityUnitTagList", { List(Int()) }, { Entity() }, GetEntityUnitTagList, true);
		Register("GetEntityListbyUnitTag", { List(Entity()) }, { Int() }, GetEntityListbyUnitTag, true);
		Register("CloseSpecifiedSoundEffectPlayer", {}, { Entity(),Int() }, CloseSpecifiedSoundEffectPlayer);
		Register("StartPauseSpecifiedSoundEffectPlayer", {}, { Entity(),Int(),Bool() }, StartPauseSpecifiedSoundEffectPlayer);
		Register("AdjustSpecifiedSoundEffectPlayer", {}, { Entity(),Int(),Int(),Float() }, AdjustSpecifiedSoundEffectPlayer);
		Register("StartPausePlayerBackgroundMusic", {}, { Entity(),Bool() }, StartPausePlayerBackgroundMusic);
		Register("AdjustPlayerBackgroundMusicVolume", {}, { Entity(),Int() }, AdjustPlayerBackgroundMusicVolume);
		Register("ModifyPlayerBackgroundMusic", {}, { Entity(),Int(),Float(),Float(),Int(),Bool(),Float(),Float(),Bool() }, ModifyPlayerBackgroundMusic);
		Register("PlayerPlaysOneShot2DSoundEffect", {}, { Entity(),Int(),Int(),Float() }, PlayerPlaysOneShot2DSoundEffect);
		Register("SettheAggroValueofSpecifiedEntity", {}, { Entity(),Entity(),Int() }, SettheAggroValueofSpecifiedEntity);
		Register("RemoveTargetEntityFromAggroList", {}, { Entity(),Entity() }, RemoveTargetEntityFromAggroList);
		Register("ClearSpecifiedTargetsAggroList", {}, { Entity() }, ClearSpecifiedTargetsAggroList);
		Register("TauntTarget", {}, { Entity(),Entity() }, TauntTarget);
		Register("QuerytheAggroValueoftheSpecifiedEntity", { Int() }, { Entity(),Entity() }, QuerytheAggroValueoftheSpecifiedEntity, true);
		Register("QuerytheAggroMultiplieroftheSpecifiedEntity", { Float() }, { Entity() }, QuerytheAggroMultiplieroftheSpecifiedEntity, true);
		Register("QueryGlobalAggroTransferMultiplier", { Float() }, {}, QueryGlobalAggroTransferMultiplier, true);
		Register("GettheAggroTargetoftheSpecifiedEntity", { Entity() }, { Entity() }, GettheAggroTargetoftheSpecifiedEntity, true);
		Register("GetListofOwnersWhoHavetheTargetinTheirAggroList", { List(Entity()) }, { Entity() }, GetListofOwnersWhoHavetheTargetinTheirAggroList, true);
		Register("GetListofOwnersThatHavetheTargetAsTheirAggroTarget", { List(Entity()) }, { Entity() }, GetListofOwnersThatHavetheTargetAsTheirAggroTarget, true);
		Register("GettheAggroListoftheSpecifiedEntity", { List(Entity()) }, { Entity() }, GettheAggroListoftheSpecifiedEntity, true);
		Register("QueryifSpecifiedEntityIsinCombat", { Bool() }, { Entity() }, QueryifSpecifiedEntityIsinCombat, true);
		Register("QueryIfFactionIsHostile", { Bool() }, { Faction(),Faction() }, QueryIfFactionIsHostile, true);
		Register("AddEntityActiveNameplate", {}, { Entity(),Cfg() }, AddEntityActiveNameplate, true);
		Register("DeleteEntityActiveNameplate", {}, { Entity(),Cfg() }, DeleteEntityActiveNameplate, true);
		Register("SetEntityActiveNameplate", {}, { Entity(),List(Cfg()) }, SetEntityActiveNameplate);
		Register("SwitchCreationPatrolTemplate", {}, { Entity(),Int() }, SwitchCreationPatrolTemplate);
		Register("SwitchActiveTextBubble", {}, { Entity(),Cfg() }, SwitchActiveTextBubble);
		Register("ModifyMiniMapZoom", {}, { Entity(),Float() }, ModifyMiniMapZoom);
		Register("ModifyMiniMapMarkerActivationStatus", {}, { Entity(),List(Int()),Bool() }, ModifyMiniMapMarkerActivationStatus);
		Register("ModifyPlayerListforVisibleMiniMapMarkers", {}, { Entity(),Int(),List(Entity()) }, ModifyPlayerListforVisibleMiniMapMarkers);
		Register("ModifyPlayerListforTrackingMiniMapMarkers", {}, { Entity(),Int(),List(Entity()) }, ModifyPlayerListforTrackingMiniMapMarkers);
		Register("ModifyPlayerMarkersontheMiniMap", {}, { Entity(),Int(),Entity() }, ModifyPlayerMarkersontheMiniMap);
		Register("CloseDeckSelector", {}, { Entity(),Int() }, CloseDeckSelector);
		Register("QueryIfAchievementIsCompleted", { Bool() }, { Entity(),Int() }, QueryIfAchievementIsCompleted, true);
		Register("SetAchievementProgressTally", {}, { Entity(),Int(),Int() }, SetAchievementProgressTally);
		Register("ChangeAchievementProgressTally", {}, { Entity(),Int(),Int() }, ChangeAchievementProgressTally);
		Register("SetPlayerSettlementRankingValue", {}, { Entity(),Int() }, SetPlayerSettlementRankingValue);
		Register("GetPlayerSettlementRankingValue", { Int() }, { Entity() }, GetPlayerSettlementRankingValue, true);
		Register("SetPlayerSettlementSuccessStatus", {}, { Entity() }, SetPlayerSettlementSuccessStatus);
		Register("GetPlayerSettlementSuccessStatus", {}, { Entity() }, GetPlayerSettlementSuccessStatus, true);
		Register("SetFactionSettlementRankingValue", {}, { Faction(),Int() }, SetFactionSettlementRankingValue);
		Register("GetFactionSettlementRankingValue", { Int() }, { Faction() }, GetFactionSettlementRankingValue, true);
		Register("SetFactionSettlementSuccessStatus", {}, { Faction() }, SetFactionSettlementSuccessStatus);
		Register("GetFactionSettlementSuccessStatus", {}, { Faction() }, GetFactionSettlementSuccessStatus, true);
		Register("GetPlayerRankScoreChange", { Int() }, { Entity() }, GetPlayerRankScoreChange, true);
		Register("SetPlayerEscapeValidity", {}, { Entity(),Bool() }, SetPlayerEscapeValidity);
		Register("GetPlayerEscapeValidity", { Bool() }, { Entity() }, GetPlayerEscapeValidity, true);
		Register("Switchthescoringgroupthataffectsplayerscompetitiverank", {}, { Entity(),Int() }, Switchthescoringgroupthataffectsplayerscompetitiverank);
		Register("SetCurrentEnvironmentTime", {}, { Float() }, SetCurrentEnvironmentTime);
		Register("SetEnvironmentTimePassageSpeed", {}, { Float() }, SetEnvironmentTimePassageSpeed);
		Register("ToggleEntityLightSource", {}, { Entity(),Int(),Bool() }, ToggleEntityLightSource);
		Register("SwitchFollowMotionDeviceTargetByEntity", {}, { Entity(),Entity(),String(),Vec(),Vec() }, SwitchFollowMotionDeviceTargetByEntity);
		Register("GetAllEntitiesWithinTheCollisionTrigger", { List(Entity()) }, { Entity(),Int() }, GetAllEntitiesWithinTheCollisionTrigger, true);
		Register("AddAffixToEquipment", {}, { Int(),Cfg(),Bool(),Float() }, AddAffixToEquipment);
		Register("RemoveEquipmentAffix", {}, { Int(),Int() }, RemoveEquipmentAffix);
		Register("ModifyEquipmentAffixValue", {}, { Int(),Int(),Float() }, ModifyEquipmentAffixValue);
		Register("GetEquipmentAffixList", { List(Int()) }, { Int() }, GetEquipmentAffixList, true);
		Register("GetEquipmentAffixConfigID", { Cfg() }, { Int(),Int() }, GetEquipmentAffixConfigID, true);
		Register("GetEquipmentAffixValue", { Float() }, { Int(),Int() }, GetEquipmentAffixValue, true);
		Register("UpdatePlayerLeaderboardScore", {}, { List(Int()),Int(),Int() }, UpdatePlayerLeaderboardScore, true);
		Register("IncreaseMaximumInventoryCapacity", {}, { Entity(),Int() }, IncreaseMaximumInventoryCapacity);
		Register("ModifyInventoryItemQuantity", {}, { Entity(),Cfg(),Int() }, ModifyInventoryItemQuantity);
		Register("SetInventoryDropItemsCurrencyAmount", {}, { Entity(),Cfg(),Int() }, SetInventoryDropItemsCurrencyAmount);
		Register("ModifyInventoryCurrencyQuantity", {}, { Entity(),Cfg(),Int() }, ModifyInventoryCurrencyQuantity);
		Register("GetInventoryCapacity", { Int() }, { Entity() }, GetInventoryCapacity, true);
		Register("GetInventoryItemQuantity", { Int() }, { Entity(),Cfg() }, GetInventoryItemQuantity, true);
		Register("GetInventoryCurrencyQuantity", { Int() }, { Entity(),Cfg() }, GetInventoryCurrencyQuantity, true);
		Register("HPLoss", {}, { Entity(),Float(),Bool(),Bool(),Bool() }, HPLoss);
		Register("RecoverHPDirectly", {}, { Entity(),Entity(),Float(),Bool(),Float(),Float(),List(String()) }, RecoverHPDirectly);
		Register("OpenShop", {}, { Entity(),Entity(),Int() }, OpenShop);
		Register("CloseShop", {}, { Entity() }, CloseShop);
		Register("RemoveItemFromCustomShopSalesList", {}, { Entity(),Int(),Int() }, RemoveItemFromCustomShopSalesList);
		Register("RemoveItemFromInventoryShopSalesList", {}, { Entity(),Int(),Cfg() }, RemoveItemFromInventoryShopSalesList);
		Register("RemoveItemFromPurchaseList", {}, { Entity(),Int(),Cfg() }, RemoveItemFromPurchaseList);
		Register("QueryCustomShopItemSalesList", { List(Int()) }, { Entity(),Int() }, QueryCustomShopItemSalesList, true);
		Register("QueryInventoryShopItemSalesList", { List(Cfg()) }, { Entity(),Int() }, QueryInventoryShopItemSalesList, true);
		Register("QueryShopPurchaseItemList", { List(Cfg()) }, { Entity(),Int() }, QueryShopPurchaseItemList, true);
		Register("GetAllEquipmentFromInventory", { List(Int()) }, { Entity() }, GetAllEquipmentFromInventory, true);
		Register("SetLootDropContent", {}, { Entity() }, SetLootDropContent);
		Register("ModifyLootItemComponentQuantity", {}, { Entity(),Cfg(),Int() }, ModifyLootItemComponentQuantity);
		Register("ModifyLootComponentCurrencyAmount", {}, { Entity(),Cfg(),Int() }, ModifyLootComponentCurrencyAmount);
		Register("GetLootComponentItemQuantity", { Int() }, { Entity(),Cfg() }, GetLootComponentItemQuantity, true);
		Register("GetLootComponentCurrencyQuantity", { Int() }, { Entity(),Cfg() }, GetLootComponentCurrencyQuantity, true);
		Register("GetAllTrophyItems", {}, { Entity() }, GetAllTrophyItems, true);
		Register("GetAllTrophyCurrency", {}, { Entity() }, GetAllTrophyCurrency, true);
		Register("GetAllEquipmentFromLootComponent", { List(Int()) }, { Entity() }, GetAllEquipmentFromLootComponent, true);
		Register("QueryEquipmentTagList", { List(Cfg()) }, { Int() }, QueryEquipmentTagList, true);
		Register("SetScanTagRules", {}, { Entity() }, SetScanTagRules);
		Register("SetScanComponentsActiveScanTagID", {}, { Entity(),Int() }, SetScanComponentsActiveScanTagID);
		Register("GetTheCurrentlyActiveScanTagConfigID", { Cfg() }, { Entity() }, GetTheCurrentlyActiveScanTagConfigID, true);
		Register("AddAffixToEquipmentAtSpecifiedID", {}, { Int(),Cfg(),Int(),Bool(),Float() }, AddAffixToEquipmentAtSpecifiedID);
		Register("RandomDeckSelectorSelectionList", {}, { List(Int()) }, RandomDeckSelectorSelectionList);
		Register("GetOwnerEntity", { Entity() }, { Entity() }, GetOwnerEntity, true);
		Register("GetListOfEntitiesOwnedByTheEntity", { List(Entity()) }, { Entity() }, GetListOfEntitiesOwnedByTheEntity, true);
		Register("QueryUnitStatusStacksBySlotID", { Int() }, { Entity(),Cfg(),Int() }, QueryUnitStatusStacksBySlotID, true);
		Register("QueryUnitStatusApplierBySlotID", { Entity() }, { Entity(),Cfg(),Int() }, QueryUnitStatusApplierBySlotID, true);
		Register("ListOfSlotIDsQueryingUnitStatus", { List(Int()) }, { Entity(),Cfg() }, ListOfSlotIDsQueryingUnitStatus, true);
		Register("QueryEquipmentConfigIDbyEquipmentID", { Cfg() }, { Int() }, QueryEquipmentConfigIDbyEquipmentID, true);
		Register("GetPlayerGUIDbyPlayerID", { Guid() }, { Int() }, GetPlayerGUIDbyPlayerID, true);
		Register("GetPlayerIDbyPlayerGUID", { Int() }, { Guid() }, GetPlayerIDbyPlayerGUID, true);
		Register("CalculateTimestampFromFormattedTime", { Int() }, { Int(),Int(),Int(),Int(),Int(),Int() }, CalculateTimestampFromFormattedTime, true);
		Register("Calculatedayoftheweekfromtimestamp", { Int() }, { Int() }, Calculatedayoftheweekfromtimestamp, true);
		Register("QueryTimestampUTC0", { Int() }, {}, QueryTimestampUTC0, true);
		Register("QueryServerTimeZone", { Int() }, {}, QueryServerTimeZone, true);
		Register("CreatePrefabGroup", { List(Entity()) }, { Int(),Vec(),Vec(),Entity(),Entity(),Int(),List(Int()),Bool() }, CreatePrefabGroup);
		Register("GetAggroListOfCreationInDefaultMode", { List(Entity()) }, { Entity() }, GetAggroListOfCreationInDefaultMode, true);
		Register("SetPlayerLeaderboardScoreAsan", {}, { List(Int()),Int(),Int() }, SetPlayerLeaderboardScoreAsanInteger);
		Register("SetPlayerLeaderboardScoreAsan", {}, { List(Int()),Float(),Int() }, SetPlayerLeaderboardScoreAsanFloat);
		Register("ModifyEnvironmentSettings", {}, { Int(),List(Entity()),Bool(),Int() }, ModifyEnvironmentSettings);
		Register("QueryGameModeAndPlayerNumber", { Int() }, {}, QueryGameModeAndPlayerNumber, true);
		Register("GetPlayerNickname", { String() }, { Entity() }, GetPlayerNickname, true);
		Register("GetPlayerClientInputDeviceType", {}, { Entity() }, GetPlayerClientInputDeviceType, true);
		Register("SetChatChannelSwitch", {}, { Int(),Bool(),Bool() }, SetChatChannelSwitch);
		Register("ModifyPlayerChannelPermission", {}, { Guid(),Int(),Bool() }, ModifyPlayerChannelPermission);
		Register("SetPlayersCurrentChannel", {}, { Guid(),List(Int()) }, SetPlayersCurrentChannel);
		Register("ConsumeGiftBox", {}, { Entity(),Int(),Int() }, ConsumeGiftBox);
		Register("QueryCorrespondingGiftBoxQuantity", { Int() }, { Entity(),Int() }, QueryCorrespondingGiftBoxQuantity, true);
		Register("QueryCorrespondingGiftBoxConsumption", { Int() }, { Entity(),Int() }, QueryCorrespondingGiftBoxConsumption, true);
		Register("WriteByBit", { Int() }, { Int(),Int(),Int(),Int() }, WriteByBit, true);
		Register("ReadByBit", { Int() }, { Int(),Int(),Int() }, ReadByBit, true);
		{
			auto generic = std::make_shared<GenericPins>();
			auto& [in_pins, out_pins] = *generic;
			in_pins[0] =
			{
				{ List(Int()), 0 },
				{ List(String()), 1 },
				{ List(Entity()), 2 },
				{ List(Guid()), 3 },
				{ List(Float()), 4 },
				{ List(Vec()), 5 },
				{ List(Bool()), 6 },
				{ List(Cfg()), 7 },
				{ List(Prefab()), 8 },
				{ List(Faction()), 9 }
			};
			in_pins[2] =
			{
				{ Int(), 0 },
				{ String(), 1 },
				{ Entity(), 2 },
				{ Guid(), 3 },
				{ Float(), 4 },
				{ Vec(), 5 },
				{ Bool(), 6 },
				{ Cfg(), 7 },
				{ Prefab(), 8 },
				{ Faction(), 9 }
			};

			Register("InsertValue", {}, { List(Int()),Int(),Int() }, InsertValueIntoListInt, false, generic);
			Register("InsertValue", {}, { List(String()),Int(),String() }, InsertValueIntoListStr, false, generic);
			Register("InsertValue", {}, { List(Entity()),Int(),Entity() }, InsertValueIntoListEntity, false, generic);
			Register("InsertValue", {}, { List(Guid()),Int(),Guid() }, InsertValueIntoListGUID, false, generic);
			Register("InsertValue", {}, { List(Float()),Int(),Float() }, InsertValueIntoListFloat, false, generic);
			Register("InsertValue", {}, { List(Vec()),Int(),Vec() }, InsertValueIntoListVec, false, generic);
			Register("InsertValue", {}, { List(Bool()),Int(),Bool() }, InsertValueIntoListBool, false, generic);
			Register("InsertValue", {}, { List(Cfg()),Int(),Cfg() }, InsertValueIntoListConfig, false, generic);
			Register("InsertValue", {}, { List(Prefab()),Int(),Prefab() }, InsertValueIntoListPrefab, false, generic);
			Register("InsertValue", {}, { List(Faction()),Int(),Faction() }, InsertValueIntoListFaction, false, generic);

			Register("SetValue", {}, { List(Int()),Int(),Int() }, ModifyValueinListInt, false, generic);
			Register("SetValue", {}, { List(String()),Int(),String() }, ModifyValueinListStr, false, generic);
			Register("SetValue", {}, { List(Entity()),Int(),Entity() }, ModifyValueinListEntity, false, generic);
			Register("SetValue", {}, { List(Guid()),Int(),Guid() }, ModifyValueinListGUID, false, generic);
			Register("SetValue", {}, { List(Float()),Int(),Float() }, ModifyValueinListFloat, false, generic);
			Register("SetValue", {}, { List(Vec()),Int(),Vec() }, ModifyValueinListVec, false, generic);
			Register("SetValue", {}, { List(Bool()),Int(),Bool() }, ModifyValueinListBool, false, generic);
			Register("SetValue", {}, { List(Cfg()),Int(),Cfg() }, ModifyValueinListConfig, false, generic);
			Register("SetValue", {}, { List(Prefab()),Int(),Prefab() }, ModifyValueinListPrefab, false, generic);
			Register("SetValue", {}, { List(Faction()),Int(),Faction() }, ModifyValueinListFaction, false, generic);
		}
		{
			auto generic = std::make_shared<GenericPins>();
			auto& [in_pins, out_pins] = *generic;
			in_pins[0] =
			{
				{ List(Int()), 0 },
				{ List(String()), 1 },
				{ List(Entity()), 2 },
				{ List(Guid()), 3 },
				{ List(Float()), 4 },
				{ List(Vec()), 5 },
				{ List(Bool()), 6 },
				{ List(Cfg()), 7 },
				{ List(Prefab()), 8 },
				{ List(Faction()), 9 }
			};

			Register("RemoveValue", {}, { List(Int()),Int() }, RemoveValueFromListInt, false, generic);
			Register("RemoveValue", {}, { List(String()),Int() }, RemoveValueFromListStr, false, generic);
			Register("RemoveValue", {}, { List(Entity()),Int() }, RemoveValueFromListEntity, false, generic);
			Register("RemoveValue", {}, { List(Guid()),Int() }, RemoveValueFromListGUID, false, generic);
			Register("RemoveValue", {}, { List(Float()),Int() }, RemoveValueFromListFloat, false, generic);
			Register("RemoveValue", {}, { List(Vec()),Int() }, RemoveValueFromListVec, false, generic);
			Register("RemoveValue", {}, { List(Bool()),Int() }, RemoveValueFromListBool, false, generic);
			Register("RemoveValue", {}, { List(Cfg()),Int() }, RemoveValueFromListConfig, false, generic);
			Register("RemoveValue", {}, { List(Prefab()),Int() }, RemoveValueFromListPrefab, false, generic);
			Register("RemoveValue", {}, { List(Faction()),Int() }, RemoveValueFromListFaction, false, generic);

			Register("Clear", {}, { List(Int()) }, ClearListInt, false, generic);
			Register("Clear", {}, { List(String()) }, ClearListStr, false, generic);
			Register("Clear", {}, { List(Entity()) }, ClearListEntity, false, generic);
			Register("Clear", {}, { List(Guid()) }, ClearListGUID, false, generic);
			Register("Clear", {}, { List(Float()) }, ClearListFloat, false, generic);
			Register("Clear", {}, { List(Vec()) }, ClearListVec, false, generic);
			Register("Clear", {}, { List(Bool()) }, ClearListBool, false, generic);
			Register("Clear", {}, { List(Cfg()) }, ClearListConfig, false, generic);
			Register("Clear", {}, { List(Prefab()) }, ClearListPrefab, false, generic);
			Register("Clear", {}, { List(Faction()) }, ClearListFaction, false, generic);
		}
	}

	std::optional<Ref> Find(const std::string& name)
	{
		auto it = registries.find(name);
		if (it == registries.end()) return {};
		return Ref(it);
	}

	static const FunctionProto& Lookup(const Ref& ref, const std::vector<Script::VarType>& args)
	{
		auto& overloads = ref.it->second;
		for (auto& proto : overloads)
		{
			if (proto.parameters.size() != args.size()) continue;
			for (int i = 0; i < args.size(); i++)
			{
				if (proto.parameters[i] != args[i]) goto next;
			}
			return proto;
		next:
			(void)0;
		}
		throw std::runtime_error("No matching overload for function: " + ref.it->first);
	}
} FunctionRegistry;

struct LValueContext
{
	LocalVar* local;

	struct
	{
		INode* entity;
		std::variant<INode*, std::string> name;
		int pin1, pin2;
	} custom;

	std::unique_ptr<INode> CreateSetter(IGraph& graph, const Script::VarType& type) const;
};

struct UserFunction
{
	std::string id;
	bool global;
};

struct ExprContent
{
	mutable std::vector<std::unique_ptr<INode>> nodes;
	Script::VarType retType{};
	mutable INode* start{};
	mutable INode* end{};
	mutable INode* flowStart{};
	mutable INode* flowEnd{};
	int pin{};
	mutable bool branch = false;
	mutable std::vector<unsigned> branches;
	std::variant<std::monostate, int64_t, float, std::string, bool> literal;
	std::variant<std::monostate, LValueContext, FunctionRegistry::Ref, std::vector<std::shared_ptr<ExprContent>>, UserFunction> extra;

	ExprContent() = default;

	explicit ExprContent(decltype(literal) literal) : literal(std::move(literal))
	{
		switch (literal.index())
		{
		case 1: retType.type = Script::VarType::Int; break;
		case 2: retType.type = Script::VarType::Float; break;
		case 3: retType.type = Script::VarType::String; break;
		case 4: retType.type = Script::VarType::Bool; break;
		}
	}

	explicit ExprContent(Script::VarType type) : retType(std::move(type)) {}

	void Add(IGraph& graph, const std::function<void(INode*)>& layout) const
	{
		for (auto& n : nodes)
		{
			auto old = n.get();
			auto ptr = &graph.AddNode(std::move(n));
			if (old == start) start = ptr;
			if (old == end) end = ptr;
			if (old == flowStart) flowStart = ptr;
			if (old == flowEnd) flowEnd = ptr;
			layout(ptr);
		}
		nodes.clear();
	}

	template<typename T>
	T Get() const
	{
		switch (literal.index())
		{
		case 1: return static_cast<T>(std::get<int64_t>(literal));
		case 2: return static_cast<T>(std::get<float>(literal));
		case 4: return static_cast<T>(std::get<bool>(literal));
		default: throw std::runtime_error("Can't convert literal");
		}
	}
};

struct ExprBuilder
{
	ExprContent& expr;

	explicit ExprBuilder(ExprContent& expr) : expr(expr)
	{
	}

	INode* Add(std::unique_ptr<INode> node) const
	{
		auto& r = expr.nodes.emplace_back(std::move(node));
		if (!expr.start) expr.start = r.get();
		return expr.end = r.get();
	}

	INode* AddFlow(std::unique_ptr<INode> node, int pin = 0) const
	{
		auto& r = expr.nodes.emplace_back(std::move(node));
		if (!expr.flowStart) expr.flowStart = r.get();
		if (!expr.start) expr.start = r.get();
		if (expr.flowEnd) expr.flowEnd->Connect(*r, pin, 0, true);
		return expr.end = expr.flowEnd = r.get();
	}

	void Combine(const ExprContent& before, int pin) const
	{
		if (before.literal.index() != 0) return;
		if (before.flowEnd && expr.flowStart) before.flowEnd->Connect(*expr.flowStart);
		if (pin >= 0 && before.end && expr.start) before.end->Connect(*expr.start, before.pin, pin);
		expr.branches.append_range(before.branches);
		if (before.branch && expr.flowStart) expr.branches.emplace_back(expr.flowStart->Id());
		expr.branch = before.branch;
		if (before.flowStart)
		{
			expr.flowStart = before.flowStart;
			if (!expr.flowEnd) expr.flowEnd = before.flowEnd;
		}
		if (before.start)
		{
			expr.start = before.start;
			if (!expr.end)
			{
				expr.end = before.end;
				expr.pin = before.pin;
			}
		}
		if (!before.nodes.empty())
		{
			auto tmp = std::move(expr.nodes);
			expr.nodes = std::move(before.nodes);
			expr.nodes.append_range(std::ranges::subrange(std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end())));
		}
		before.start = before.end = before.flowStart = before.flowEnd = nullptr;
		before.branch = false;
		before.branches.clear();
	}
};

struct NodeFactory
{
	using enum NodeId;

	static std::unique_ptr<INode> GetLocalVariable(IGraph& graph, const Script::VarType& type)
	{
		auto id = GetLocalVariableBool;
		unsigned index = 0;
		switch (type.type)
		{
		case Script::VarType::Int:
			id = GetLocalVariableInt;
			index = 1;
			break;
		case Script::VarType::Float:
			id = GetLocalVariableFloat;
			index = 5;
			break;
		case Script::VarType::String:
			id = GetLocalVariableStr;
			index = 2;
			break;
		case Script::VarType::Bool:
			id = GetLocalVariableBool;
			index = 0;
			break;
		case Script::VarType::Entity:
			id = GetLocalVariableEntity;
			index = 3;
			break;
		case Script::VarType::Vec:
			id = GetLocalVariableVec;
			index = 6;
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(type.extra))
			{
			case GuidEx::Entity:
				id = GetLocalVariableGUID;
				index = 4;
				break;
			case GuidEx::Prefab:
				id = GetLocalVariablePrefab;
				index = 15;
				break;
			case GuidEx::Configuration:
				id = GetLocalVariableConfig;
				index = 14;
				break;
			case GuidEx::Faction:
				id = GetLocalVariableFaction;
				index = 18;
				break;
			}
			break;
		case Script::VarType::List:
			switch (std::any_cast<Script::VarType>(type.extra).type)
			{
			case Script::VarType::Int:
				id = GetLocalVariableListInt;
				index = 7;
				break;
			case Script::VarType::Float:
				id = GetLocalVariableListFloat;
				index = 11;
				break;
			case Script::VarType::String:
				id = GetLocalVariableListStr;
				index = 8;
				break;
			case Script::VarType::Bool:
				id = GetLocalVariableListBool;
				index = 13;
				break;
			case Script::VarType::Entity:
				id = GetLocalVariableListEntity;
				index = 9;
				break;
			case Script::VarType::Vec:
				id = GetLocalVariableListVec;
				index = 12;
				break;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					id = GetLocalVariableListGUID;
					index = 10;
					break;
				case GuidEx::Prefab:
					id = GetLocalVariableListPrefab;
					index = 17;
					break;
				case GuidEx::Configuration:
					id = GetLocalVariableListConfig;
					index = 16;
					break;
				case GuidEx::Faction:
					id = GetLocalVariableListFaction;
					index = 19;
					break;
				}
				break;
			default: throw std::runtime_error("Unsupported variable type");
			}
			break;
		default: throw std::runtime_error("Unsupported variable type");
		}
		auto node = graph.CreateNode(id);
		node->Set(0, index, false);
		node->Set(1, index, true);
		return node;
	}

	static std::unique_ptr<INode> SetLocalVariable(IGraph& graph, const Script::VarType& type)
	{
		auto id = SetLocalVariableBool;
		unsigned index = 0;
		switch (type.type)
		{
		case Script::VarType::Int:
			id = SetLocalVariableInt;
			index = 1;
			break;
		case Script::VarType::Float:
			id = SetLocalVariableFloat;
			index = 5;
			break;
		case Script::VarType::String:
			id = SetLocalVariableStr;
			index = 2;
			break;
		case Script::VarType::Bool:
			id = SetLocalVariableBool;
			index = 0;
			break;
		case Script::VarType::Entity:
			id = SetLocalVariableEntity;
			index = 3;
			break;
		case Script::VarType::Vec:
			id = SetLocalVariableVec;
			index = 6;
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(type.extra))
			{
			case GuidEx::Entity:
				id = SetLocalVariableGUID;
				index = 4;
				break;
			case GuidEx::Prefab:
				id = SetLocalVariablePrefab;
				index = 15;
				break;
			case GuidEx::Configuration:
				id = SetLocalVariableConfig;
				index = 14;
				break;
			case GuidEx::Faction:
				id = SetLocalVariableFaction;
				index = 18;
				break;
			}
			break;
		case Script::VarType::List:
			switch (std::any_cast<Script::VarType>(type.extra).type)
			{
			case Script::VarType::Int:
				id = SetLocalVariableListInt;
				index = 7;
				break;
			case Script::VarType::Float:
				id = SetLocalVariableListFloat;
				index = 11;
				break;
			case Script::VarType::String:
				id = SetLocalVariableListStr;
				index = 8;
				break;
			case Script::VarType::Bool:
				id = SetLocalVariableListBool;
				index = 13;
				break;
			case Script::VarType::Entity:
				id = SetLocalVariableListEntity;
				index = 9;
				break;
			case Script::VarType::Vec:
				id = SetLocalVariableListVec;
				index = 12;
				break;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					id = SetLocalVariableListGUID;
					index = 10;
					break;
				case GuidEx::Prefab:
					id = SetLocalVariableListPrefab;
					index = 17;
					break;
				case GuidEx::Configuration:
					id = SetLocalVariableListConfig;
					index = 16;
					break;
				case GuidEx::Faction:
					id = SetLocalVariableListFaction;
					index = 19;
					break;
				}
				break;
			default: throw std::runtime_error("Unsupported variable type");
			}
			break;
		default: throw std::runtime_error("Unsupported variable type");
		}
		auto node = graph.CreateNode(id);
		node->Set(1, index, false);
		return node;
	}

	static void GetCustomVariable(INode* node, const Script::VarType& type)
	{
		auto id = GetCustomVariableBool;
		unsigned index = 0;
		switch (type.type)
		{
		case Script::VarType::Int:
			id = GetCustomVariableInt;
			index = 0;
			break;
		case Script::VarType::Float:
			id = GetCustomVariableFloat;
			index = 4;
			break;
		case Script::VarType::String:
			id = GetCustomVariableStr;
			index = 1;
			break;
		case Script::VarType::Bool:
			id = GetCustomVariableBool;
			index = 6;
			break;
		case Script::VarType::Entity:
			id = GetCustomVariableEntity;
			index = 2;
			break;
		case Script::VarType::Vec:
			id = GetCustomVariableVec;
			index = 5;
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(type.extra))
			{
			case GuidEx::Entity:
				id = GetCustomVariableGUID;
				index = 3;
				break;
			case GuidEx::Prefab:
				id = GetCustomVariablePrefab;
				index = 15;
				break;
			case GuidEx::Configuration:
				id = GetCustomVariableConfig;
				index = 14;
				break;
			case GuidEx::Faction:
				id = GetCustomVariableFaction;
				index = 18;
				break;
			}
			break;
		case Script::VarType::List:
			switch (std::any_cast<Script::VarType>(type.extra).type)
			{
			case Script::VarType::Int:
				id = GetCustomVariableListInt;
				index = 7;
				break;
			case Script::VarType::Float:
				id = GetCustomVariableListFloat;
				index = 11;
				break;
			case Script::VarType::String:
				id = GetCustomVariableListStr;
				index = 8;
				break;
			case Script::VarType::Bool:
				id = GetCustomVariableListBool;
				index = 13;
				break;
			case Script::VarType::Entity:
				id = GetCustomVariableListEntity;
				index = 9;
				break;
			case Script::VarType::Vec:
				id = GetCustomVariableListVec;
				index = 12;
				break;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					id = GetCustomVariableListGUID;
					index = 10;
					break;
				case GuidEx::Prefab:
					id = GetCustomVariableListPrefab;
					index = 17;
					break;
				case GuidEx::Configuration:
					id = GetCustomVariableListConfig;
					index = 16;
					break;
				case GuidEx::Faction:
					id = GetCustomVariableListFaction;
					index = 19;
					break;
				}
				break;
			default: throw std::runtime_error("Unsupported variable type");
			}
			break;
		default: throw std::runtime_error("Unsupported variable type");
		}
		node->Change(id);
		node->Set(0, index, true);
	}

	static std::unique_ptr<INode> GetCustomVariable(IGraph& graph, const Script::VarType& type)
	{
		auto node = graph.CreateNode(GetCustomVariableBool);
		if (type.type != Script::VarType::Unknown) GetCustomVariable(node.get(), type);
		return node;
	}

	static std::unique_ptr<INode> SetCustomVariable(IGraph& graph, const Script::VarType& type)
	{
		auto id = SetCustomVariableBool;
		unsigned index = 0;
		switch (type.type)
		{
		case Script::VarType::Int:
			id = SetCustomVariableInt;
			index = 0;
			break;
		case Script::VarType::Float:
			id = SetCustomVariableFloat;
			index = 4;
			break;
		case Script::VarType::String:
			id = SetCustomVariableStr;
			index = 1;
			break;
		case Script::VarType::Bool:
			id = SetCustomVariableBool;
			index = 6;
			break;
		case Script::VarType::Entity:
			id = SetCustomVariableEntity;
			index = 2;
			break;
		case Script::VarType::Vec:
			id = SetCustomVariableVec;
			index = 5;
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(type.extra))
			{
			case GuidEx::Entity:
				id = SetCustomVariableGUID;
				index = 3;
				break;
			case GuidEx::Prefab:
				id = SetCustomVariablePrefab;
				index = 15;
				break;
			case GuidEx::Configuration:
				id = SetCustomVariableConfig;
				index = 14;
				break;
			case GuidEx::Faction:
				id = SetCustomVariableFaction;
				index = 18;
				break;
			}
			break;
		case Script::VarType::List:
			switch (std::any_cast<Script::VarType>(type.extra).type)
			{
			case Script::VarType::Int:
				id = SetCustomVariableListInt;
				index = 7;
				break;
			case Script::VarType::Float:
				id = SetCustomVariableListFloat;
				index = 11;
				break;
			case Script::VarType::String:
				id = SetCustomVariableListStr;
				index = 8;
				break;
			case Script::VarType::Bool:
				id = SetCustomVariableListBool;
				index = 13;
				break;
			case Script::VarType::Entity:
				id = SetCustomVariableListEntity;
				index = 9;
				break;
			case Script::VarType::Vec:
				id = SetCustomVariableListVec;
				index = 12;
				break;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					id = SetCustomVariableListGUID;
					index = 10;
					break;
				case GuidEx::Prefab:
					id = SetCustomVariableListPrefab;
					index = 17;
					break;
				case GuidEx::Configuration:
					id = SetCustomVariableListConfig;
					index = 16;
					break;
				case GuidEx::Faction:
					id = SetCustomVariableListFaction;
					index = 19;
					break;
				}
				break;
			default: throw std::runtime_error("Unsupported variable type");
			}
			break;
		default: throw std::runtime_error("Unsupported variable type");
		}
		auto node = graph.CreateNode(id);
		node->Set(2, index, false);
		return node;
	}

	static std::unique_ptr<INode> Add(IGraph& graph, const ExprContent& e1, const ExprContent& e2)
	{
		auto check = [](Script::VarType::Type type)
			{
				switch (type)
				{
				case Script::VarType::Int:
				case Script::VarType::Float:
				case Script::VarType::Vec:
					break;
				default: throw std::runtime_error("Unsupported variable type for addition");
				}
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for addition");
		check(e1.retType.type);
		check(e2.retType.type);
		switch (e1.retType.type)
		{
		case Script::VarType::Int:
		{
			auto node = graph.CreateNode(AdditionInt);
			node->Set(0, 0, false);
			node->Set(1, 0, false);
			node->Set(0, 0, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<int64_t>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<int64_t>(e2.literal));
			return node;
		}
		case Script::VarType::Float:
		{
			auto node = graph.CreateNode(AdditionFloat);
			node->Set(0, 1, false);
			node->Set(1, 1, false);
			node->Set(0, 1, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<float>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<float>(e2.literal));
			return node;
		}
		case Script::VarType::Vec:
			return graph.CreateNode(_3DVectorAddition);
		default: break;
		}
		throw std::runtime_error("Unreachable code");
	}

	static std::unique_ptr<INode> Sub(IGraph& graph, const ExprContent& e1, const ExprContent& e2)
	{
		auto check = [](Script::VarType::Type type)
			{
				switch (type)
				{
				case Script::VarType::Int:
				case Script::VarType::Float:
				case Script::VarType::Vec:
					break;
				default: throw std::runtime_error("Unsupported variable type for subtraction");
				}
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for subtraction");
		check(e1.retType.type);
		check(e2.retType.type);
		switch (e1.retType.type)
		{
		case Script::VarType::Int:
		{
			auto node = graph.CreateNode(SubtractionInt);
			node->Set(0, 0, false);
			node->Set(1, 0, false);
			node->Set(0, 0, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<int64_t>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<int64_t>(e2.literal));
			return node;
		}
		case Script::VarType::Float:
		{
			auto node = graph.CreateNode(SubtractionFloat);
			node->Set(0, 1, false);
			node->Set(1, 1, false);
			node->Set(0, 1, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<float>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<float>(e2.literal));
			return node;
		}
		case Script::VarType::Vec:
			return graph.CreateNode(_3DVectorSubtraction);
		default: break;
		}
		throw std::runtime_error("Unreachable code");
	}

	static std::unique_ptr<INode> Mul(IGraph& graph, const ExprContent& e1, const ExprContent& e2)
	{
		auto check = [](Script::VarType::Type type)
			{
				switch (type)
				{
				case Script::VarType::Int:
				case Script::VarType::Float:
				case Script::VarType::Vec:
					break;
				default: throw std::runtime_error("Unsupported variable type for multiplication");
				}
			};
		if (e1.retType.type != e2.retType.type && !(e1.retType.type == Script::VarType::Vec && e2.retType.type == Script::VarType::Float)) throw std::runtime_error("Type mismatch for multiplication");
		check(e1.retType.type);
		check(e2.retType.type);
		switch (e1.retType.type)
		{
		case Script::VarType::Int:
		{
			auto node = graph.CreateNode(MultiplicationInt);
			node->Set(0, 0, false);
			node->Set(1, 0, false);
			node->Set(0, 0, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<int64_t>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<int64_t>(e2.literal));
			return node;
		}
		case Script::VarType::Float:
		{
			auto node = graph.CreateNode(MultiplicationFloat);
			node->Set(0, 1, false);
			node->Set(1, 1, false);
			node->Set(0, 1, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<float>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<float>(e2.literal));
			return node;
		}
		case Script::VarType::Vec:
		{
			auto node = graph.CreateNode(_3DVectorZoom);
			if (e2.literal.index() != 0) node->Set(1, std::get<float>(e2.literal));
			return node;
		}
		default: break;
		}
		throw std::runtime_error("Unreachable code");
	}

	static std::unique_ptr<INode> Div(IGraph& graph, const ExprContent& e1, const ExprContent& e2)
	{
		auto check = [](Script::VarType::Type type)
			{
				switch (type)
				{
				case Script::VarType::Int:
				case Script::VarType::Float:
					break;
				default: throw std::runtime_error("Unsupported variable type for division");
				}
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for division");
		check(e1.retType.type);
		check(e2.retType.type);
		switch (e1.retType.type)
		{
		case Script::VarType::Int:
		{
			auto node = graph.CreateNode(DivisionInt);
			node->Set(0, 0, false);
			node->Set(1, 0, false);
			node->Set(0, 0, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<int64_t>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<int64_t>(e2.literal));
			return node;
		}
		case Script::VarType::Float:
		{
			auto node = graph.CreateNode(DivisionFloat);
			node->Set(0, 1, false);
			node->Set(1, 1, false);
			node->Set(0, 1, true);
			if (e1.literal.index() != 0) node->Fill(0, std::get<float>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<float>(e2.literal));
			return node;
		}
		default: break;
		}
		throw std::runtime_error("Unreachable code");
	}

	static std::unique_ptr<INode> Mod(IGraph& graph, const ExprContent& e1, const ExprContent& e2)
	{
		auto check = [](Script::VarType::Type type)
			{
				if (type != Script::VarType::Int) throw std::runtime_error("Unsupported variable type for modulo");
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for modulo");
		check(e1.retType.type);
		check(e2.retType.type);
		auto node = graph.CreateNode(ModuloOperation);
		if (e1.literal.index() != 0) node->Set(0, (uint64_t)std::get<int64_t>(e1.literal));
		if (e2.literal.index() != 0) node->Set(1, (uint64_t)std::get<int64_t>(e2.literal));
		return node;
	}

	static std::unique_ptr<INode> Compare(IGraph& graph, const ExprContent& e1, const ExprContent& e2, BinaryExpr::Op op)
	{
		auto check = [](Script::VarType::Type type)
			{
				switch (type)
				{
				case Script::VarType::Int:
				case Script::VarType::Float:
					break;
				default: throw std::runtime_error("Unsupported variable type for compare");
				}
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for compare");
		check(e1.retType.type);
		check(e2.retType.type);
		switch (e1.retType.type)
		{
		case Script::VarType::Int:
		{
			NodeId id;
			switch (op)
			{
			case BinaryExpr::LT:
				id = LessThanInt;
				break;
			case BinaryExpr::GT:
				id = GreaterThanInt;
				break;
			case BinaryExpr::LE:
				id = LessThanorEqualToInt;
				break;
			case BinaryExpr::GE:
				id = GreaterThanorEqualToInt;
				break;
			default: throw std::runtime_error("Unexcepted argument");
			}
			auto node = graph.CreateNode(id);
			node->Set(0, 0, false);
			node->Set(1, 0, false);
			if (e1.literal.index() != 0) node->Fill(0, std::get<int64_t>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<int64_t>(e2.literal));
			return node;
		}
		case Script::VarType::Float:
		{
			NodeId id;
			switch (op)
			{
			case BinaryExpr::LT:
				id = LessThanFloat;
				break;
			case BinaryExpr::GT:
				id = GreaterThanFloat;
				break;
			case BinaryExpr::LE:
				id = LessThanorEqualToFloat;
				break;
			case BinaryExpr::GE:
				id = GreaterThanorEqualToFloat;
				break;
			default: throw std::runtime_error("Unexcepted argument");
			}
			auto node = graph.CreateNode(id);
			node->Set(0, 1, false);
			node->Set(1, 1, false);
			if (e1.literal.index() != 0) node->Fill(0, std::get<float>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<float>(e2.literal));
			return node;
		}
		default: break;
		}
		throw std::runtime_error("Unreachable code");
	}

	static std::unique_ptr<INode> Equal(IGraph& graph, const ExprContent& e1, const ExprContent& e2)
	{
		auto check = [](Script::VarType::Type type)
			{
				switch (type)
				{
				case Script::VarType::Int:
				case Script::VarType::Float:
				case Script::VarType::String:
				case Script::VarType::Vec:
				case Script::VarType::Entity:
				case Script::VarType::Guid:
					break;
				default: throw std::runtime_error("Unsupported variable type for equal");
				}
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for equal");
		check(e1.retType.type);
		check(e2.retType.type);
		switch (e1.retType.type)
		{
		case Script::VarType::Int:
		{
			auto node = graph.CreateNode(EqualInt);
			node->Set(0, 5, false);
			node->Set(1, 5, false);
			if (e1.literal.index() != 0) node->Fill(0, e1.Get<int64_t>());
			if (e2.literal.index() != 0) node->Fill(1, e2.Get<int64_t>());
			return node;
		}
		case Script::VarType::Float:
		{
			auto node = graph.CreateNode(EqualFloat);
			node->Set(0, 6, false);
			node->Set(1, 6, false);
			if (e1.literal.index() != 0) node->Fill(0, e1.Get<float>());
			if (e2.literal.index() != 0) node->Fill(1, e2.Get<float>());
			return node;
		}
		case Script::VarType::String:
		{
			auto node = graph.CreateNode(EqualStr);
			node->Set(0, 0, false);
			node->Set(1, 0, false);
			if (e1.literal.index() != 0) node->Fill(0, std::get<std::string>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<std::string>(e2.literal));
			return node;
		}
		case Script::VarType::Vec:
		{
			auto node = graph.CreateNode(EqualVec);
			node->Set(0, 3, false);
			node->Set(1, 3, false);
			if (e1.literal.index() != 0) node->Fill(0, std::get<float>(e1.literal));
			if (e2.literal.index() != 0) node->Fill(1, std::get<float>(e2.literal));
			return node;
		}
		case Script::VarType::Guid:
		{
			switch (std::any_cast<GuidEx>(e1.retType.extra))
			{
			case GuidEx::Entity:
			{
				auto node = graph.CreateNode(EqualEntity);
				node->Set(0, 1, false);
				node->Set(1, 1, false);
				if (e1.literal.index() != 0) node->Fill(0, GUID{ (unsigned)e1.Get<int64_t>() }, ServerVarType::GUID);
				if (e2.literal.index() != 0) node->Fill(1, GUID{ (unsigned)e2.Get<int64_t>() }, ServerVarType::GUID);
				return node;
			}
			case GuidEx::Prefab:
			{
				auto node = graph.CreateNode(EqualPrefab);
				node->Set(0, 8, false);
				node->Set(1, 8, false);
				if (e1.literal.index() != 0) node->Fill(0, GUID{ (unsigned)e1.Get<int64_t>() }, ServerVarType::Prefab);
				if (e2.literal.index() != 0) node->Fill(1, GUID{ (unsigned)e2.Get<int64_t>() }, ServerVarType::Prefab);
				return node;
			}
			case GuidEx::Configuration:
			{
				auto node = graph.CreateNode(EqualConfig);
				node->Set(0, 7, false);
				node->Set(1, 7, false);
				if (e1.literal.index() != 0) node->Fill(0, GUID{ (unsigned)e1.Get<int64_t>() }, ServerVarType::Configuration);
				if (e2.literal.index() != 0) node->Fill(1, GUID{ (unsigned)e2.Get<int64_t>() }, ServerVarType::Configuration);
				return node;
			}
			case GuidEx::Faction:
				break;
			}
		}
		case Script::VarType::Entity:
		{
			auto node = graph.CreateNode(EqualEntity);
			node->Set(0, 2, false);
			node->Set(1, 2, false);
			if (e1.literal.index() != 0) node->Fill(0, e1.Get<int64_t>());
			if (e2.literal.index() != 0) node->Fill(1, e2.Get<int64_t>());
			return node;
		}
		case Script::VarType::Bool:
		{
			auto node = graph.CreateNode(EqualBool);
			node->Set(0, 9, false);
			node->Set(1, 9, false);
			if (e1.literal.index() != 0) node->Fill(0, Enum{ (unsigned)e1.Get<int64_t>() }, ServerVarType::Boolean);
			if (e2.literal.index() != 0) node->Fill(1, Enum{ (unsigned)e2.Get<int64_t>() }, ServerVarType::Boolean);
			return node;
		}
		default: break;
		}
		throw std::runtime_error("Unreachable code");
	}

	static std::unique_ptr<INode> Not(IGraph& graph, const ExprContent& expr)
	{
		if (expr.retType.type != Script::VarType::Bool) throw std::runtime_error("Unsupported variable type for logical NOT");
		auto node = graph.CreateNode(LogicalNOTOperation);
		if (expr.literal.index() != 0) node->Set(0, Enum{ (unsigned)std::get<int64_t>(expr.literal) }, ServerVarType::Boolean);
		return node;
	}

	static std::unique_ptr<INode> Bitwise(IGraph& graph, const ExprContent& e1, const ExprContent& e2, BinaryExpr::Op op)
	{
		auto check = [](Script::VarType::Type type)
			{
				if (type != Script::VarType::Int) throw std::runtime_error("Unsupported variable type for bitwise");
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for bitwise");
		check(e1.retType.type);
		check(e2.retType.type);
		NodeId id;
		switch (op)
		{
		case BinaryExpr::AND:
			id = BitwiseAND;
			break;
		case BinaryExpr::OR:
			id = BitwiseOR;
			break;
		case BinaryExpr::XOR:
			id = XORExclusiveOR;
			break;
		case BinaryExpr::ShL:
			id = LeftShiftOperation;
			break;
		case BinaryExpr::ShR:
		case BinaryExpr::ShA:
			id = RightShiftOperation;
			break;
		default: throw std::runtime_error("Unexcepted argument");
		}
		auto node = graph.CreateNode(id);
		if (e1.literal.index() != 0) node->Set(0, (uint64_t)std::get<int64_t>(e1.literal));
		if (e2.literal.index() != 0) node->Set(1, (uint64_t)std::get<int64_t>(e2.literal));
		return node;
	}

	static std::unique_ptr<INode> Logical(IGraph& graph, const ExprContent& e1, const ExprContent& e2, BinaryExpr::Op op)
	{
		auto check = [](Script::VarType::Type type)
			{
				if (type != Script::VarType::Bool) throw std::runtime_error("Unsupported variable type for logical");
			};
		if (e1.retType.type != e2.retType.type) throw std::runtime_error("Type mismatch for logical");
		check(e1.retType.type);
		check(e2.retType.type);
		NodeId id;
		switch (op)
		{
		case BinaryExpr::LogAND:
			id = LogicalANDOperation;
			break;
		case BinaryExpr::LogOR:
			id = LogicalOROperation;
			break;
		case BinaryExpr::LogXOR:
			id = LogicalXOROperation;
			break;
		default: throw std::runtime_error("Unexcepted argument");
		}
		auto node = graph.CreateNode(id);
		if (e1.literal.index() != 0) node->Set(0, Enum{ (unsigned)std::get<int64_t>(e1.literal) }, ServerVarType::Boolean);
		if (e2.literal.index() != 0) node->Set(1, Enum{ (unsigned)std::get<int64_t>(e1.literal) }, ServerVarType::Boolean);
		return node;
	}

	static std::unique_ptr<INode> BitwiseNot(IGraph& graph, const ExprContent& expr)
	{
		if (expr.retType.type != Script::VarType::Int) throw std::runtime_error("Unsupported variable type for bitwise NOT");
		auto node = graph.CreateNode(BitwiseComplement);
		if (expr.literal.index() != 0) node->Set(0, (uint64_t)std::get<int64_t>(expr.literal));
		return node;
	}

	static std::unique_ptr<INode> Cast(IGraph& graph, const ExprContent& expr, const Script::VarType& type)
	{
		switch (expr.retType.type)
		{
		case Script::VarType::Int:
			switch (type.type)
			{
			case Script::VarType::Float:
			case Script::VarType::Bool:
			case Script::VarType::String:
				goto next;
			default: throw std::runtime_error("Unsupported target type for cast");
			}
		case Script::VarType::Float:
			switch (type.type)
			{
			case Script::VarType::Int:
			case Script::VarType::String:
				goto next;
			default: throw std::runtime_error("Unsupported target type for cast");
			}
		case Script::VarType::Bool:
			switch (type.type)
			{
			case Script::VarType::Int:
			case Script::VarType::String:
				goto next;
			default: throw std::runtime_error("Unsupported target type for cast");
			}
		case Script::VarType::Vec:
		case Script::VarType::Entity:
			if (type.type != Script::VarType::String) throw std::runtime_error("Unsupported target type for cast");
			goto next;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(expr.retType.extra))
			{
			case GuidEx::Entity:
			case GuidEx::Faction:
				if (type.type != Script::VarType::String) throw std::runtime_error("Unsupported target type for cast");
				goto next;
			default: throw std::runtime_error("Unsupported variable type for cast");
			}
		default: throw std::runtime_error("Unsupported variable type for cast");
		}
	next:
		unsigned in, out;
		switch (expr.retType.type)
		{
		case Script::VarType::Int:
			in = 0;
			break;
		case Script::VarType::Float:
			in = 4;
			break;
		case Script::VarType::Bool:
			in = 3;
			break;
		case Script::VarType::Entity:
			in = 1;
			break;
		case Script::VarType::Vec:
			in = 5;
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(expr.retType.extra))
			{
			case GuidEx::Entity:
				in = 2;
				break;
			case GuidEx::Faction:
				in = 6;
				break;
			default: throw std::exception("Unreached code");
			}
			break;
		default: throw std::exception("Unreached code");
		}
		switch (type.type)
		{
		case Script::VarType::Int:
			out = 3;
			break;
		case Script::VarType::Float:
			out = 1;
			break;
		case Script::VarType::Bool:
			out = 0;
			break;
		case Script::VarType::String:
			out = 2;
			break;
		default:throw std::exception("Unreached code");
		}
		auto node = graph.CreateNode(DataTypeConversionIntBool);
		node->Set(0, in, false);
		node->Set(0, out, true);
		switch (expr.literal.index())
		{
		case 1:
			node->Fill(0, std::get<int64_t>(expr.literal));
			break;
		case 2:
			node->Fill(0, std::get<float>(expr.literal));
			break;
		}
		return node;
	}
};

struct VarContent
{
	std::variant<INode*, unsigned> content;
	bool iterator;
};

std::unique_ptr<INode> LValueContext::CreateSetter(IGraph& graph, const Script::VarType& type) const
{
	std::unique_ptr<INode> node;
	if (local)
	{
		node = NodeFactory::SetLocalVariable(graph, local->type);
		auto& [content, iterator] = std::any_cast<VarContent&>(local->content);
		std::get<0>(content)->Connect(*node, 0, 0);
	}
	else
	{
		node = NodeFactory::SetCustomVariable(graph, type);
		custom.entity->Connect(*node, custom.pin1, 0);
		if (custom.name.index() == 0) std::get<0>(custom.name)->Connect(*node, custom.pin2, 1);
		else node->Set(1, std::get<1>(custom.name));
	}
	return node;
}

static const std::unordered_set<std::string> KEYWORDS
{
	"int","float","bool","string","entity","vec","faction","guid","prefab","cfg","list","map",
	"if","else","for","while","switch","case","default","return","break","signal","struct",
	"var","void","global","event","as",
	"null","true","false","this"
};

static void DefinePin(INode& node, const Script::VarType& type, int pin, bool generic, bool out)
{
	if (generic)
	{
		switch (type.type)
		{
		case Script::VarType::Int:
			node.Set(pin, ServerVarType::Integer, -1, out);
			break;
		case Script::VarType::Float:
			node.Set(pin, ServerVarType::Float, -1, out);
			break;
		case Script::VarType::String:
			node.Set(pin, ServerVarType::String, -1, out);
			break;
		case Script::VarType::Bool:
			node.Set(pin, ServerVarType::Boolean, -1, out);
			break;
		case Script::VarType::Entity:
			node.Set(pin, ServerVarType::Entity, -1, out);
			break;
		case Script::VarType::Vec:
			node.Set(pin, ServerVarType::Vector, -1, out);
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(type.extra))
			{
			case GuidEx::Entity:
				node.Set(pin, ServerVarType::GUID, -1, out);
				break;
			case GuidEx::Prefab:
				node.Set(pin, ServerVarType::Prefab, -1, out);
				break;
			case GuidEx::Configuration:
				node.Set(pin, ServerVarType::Configuration, -1, out);
				break;
			case GuidEx::Faction:
				node.Set(pin, ServerVarType::Faction, -1, out);
				break;
			}
			break;
		case Script::VarType::List:
			switch (std::any_cast<Script::VarType>(type.extra).type)
			{
			case Script::VarType::Int:
				node.Set(pin, ServerVarType::IntegerList, -1, out);
				break;
			case Script::VarType::Float:
				node.Set(pin, ServerVarType::FloatList, -1, out);
				break;
			case Script::VarType::String:
				node.Set(pin, ServerVarType::StringList, -1, out);
				break;
			case Script::VarType::Bool:
				node.Set(pin, ServerVarType::BooleanList, -1, out);
				break;
			case Script::VarType::Entity:
				node.Set(pin, ServerVarType::EntityList, -1, out);
				break;
			case Script::VarType::Vec:
				node.Set(pin, ServerVarType::VectorList, -1, out);
				break;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					node.Set(pin, ServerVarType::GUIDList, -1, out);
					break;
				case GuidEx::Prefab:
					node.Set(pin, ServerVarType::PrefabList, -1, out);
					break;
				case GuidEx::Configuration:
					node.Set(pin, ServerVarType::ConfigurationList, -1, out);
					break;
				case GuidEx::Faction:
					node.Set(pin, ServerVarType::FactionList, -1, out);
					break;
				}
				break;
			default: throw std::runtime_error("Type unsupported");
			}
			break;
		default: throw std::runtime_error("Type unsupported");
		}
		return;
	}
	switch (type.type)
	{
	case Script::VarType::Int:
		node.Set(pin, ServerVarType::Integer, out);
		break;
	case Script::VarType::Float:
		node.Set(pin, ServerVarType::Float, out);
		break;
	case Script::VarType::String:
		node.Set(pin, ServerVarType::String, out);
		break;
	case Script::VarType::Bool:
		node.Set(pin, ServerVarType::Boolean, out);
		break;
	case Script::VarType::Entity:
		node.Set(pin, ServerVarType::Entity, out);
		break;
	case Script::VarType::Vec:
		node.Set(pin, ServerVarType::Vector, out);
		break;
	case Script::VarType::Guid:
		switch (std::any_cast<GuidEx>(type.extra))
		{
		case GuidEx::Entity:
			node.Set(pin, ServerVarType::GUID, out);
			break;
		case GuidEx::Prefab:
			node.Set(pin, ServerVarType::Prefab, out);
			break;
		case GuidEx::Configuration:
			node.Set(pin, ServerVarType::Configuration, out);
			break;
		case GuidEx::Faction:
			node.Set(pin, ServerVarType::Faction, out);
			break;
		}
		break;
	case Script::VarType::List:
		switch (std::any_cast<Script::VarType>(type.extra).type)
		{
		case Script::VarType::Int:
			node.Set(pin, ServerVarType::IntegerList, out);
			break;
		case Script::VarType::Float:
			node.Set(pin, ServerVarType::FloatList, out);
			break;
		case Script::VarType::String:
			node.Set(pin, ServerVarType::StringList, out);
			break;
		case Script::VarType::Bool:
			node.Set(pin, ServerVarType::BooleanList, out);
			break;
		case Script::VarType::Entity:
			node.Set(pin, ServerVarType::EntityList, out);
			break;
		case Script::VarType::Vec:
			node.Set(pin, ServerVarType::VectorList, out);
			break;
		case Script::VarType::Guid:
			switch (std::any_cast<GuidEx>(type.extra))
			{
			case GuidEx::Entity:
				node.Set(pin, ServerVarType::GUIDList, out);
				break;
			case GuidEx::Prefab:
				node.Set(pin, ServerVarType::PrefabList, out);
				break;
			case GuidEx::Configuration:
				node.Set(pin, ServerVarType::ConfigurationList, out);
				break;
			case GuidEx::Faction:
				node.Set(pin, ServerVarType::FactionList, out);
				break;
			}
			break;
		default: throw std::runtime_error("Type unsupported");
		}
		break;
	default: throw std::runtime_error("Type unsupported");
	}
}

class NodeGenerator : public ASTVisitor
{
	friend Compiler;
	using enum NodeId;
	IGraph& graph;
	Compiler& compiler;
	INode* prev = nullptr;
	INode* entrypoint = nullptr;
	INode* current_loop = nullptr;
	std::optional<std::reference_wrapper<const EventProto>> proto;
	float x = 0, y = 0;
	unsigned flow = 0;

	struct
	{
		INode* ret;
		std::optional<Script::VarType> type;
		bool in_function;
	} function_header{};

	struct
	{
		struct Arg
		{
			INode* node;
			Script::VarType type;
		};

		struct Declaration
		{
			std::vector<Arg> parameters;
			std::optional<Arg> ret;
			INode* entrypoint;
		};

		std::unordered_map<std::string, Declaration> map;
	} function_storage;
public:
	explicit NodeGenerator(IGraph& graph, Compiler& compiler) : graph(graph), compiler(compiler)
	{
	}

private:
	void AutoLayout(INode* node)
	{
		node->SetPos(x, y);
		x += 500;
		if (x > 2000)
		{
			x = 0;
			y += 600;
		}
	}

	static void SetLiteral(INode& node, int pin, const ExprContent& expr, const Script::VarType& type)
	{
		switch (expr.literal.index())
		{
		case 0: return;
		case 1:
		case 2:
			switch (type.type)
			{
			case Script::VarType::Bool:
				node.Fill(pin, Enum{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Boolean);
				return;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					node.Fill(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::GUID);
					break;
				case GuidEx::Prefab:
					node.Fill(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Prefab);
					break;
				case GuidEx::Configuration:
					node.Fill(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Configuration);
					break;
				case GuidEx::Faction:
					node.Fill(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Faction);
					break;
				}
				return;
			case Script::VarType::Int:
				node.Fill(pin, expr.Get<int64_t>());
				return;
			case Script::VarType::Float:
				node.Fill(pin, expr.Get<float>());
				return;
			default: throw std::runtime_error("Type mismatch");
			}
		case 3:
			node.Fill(pin, std::get<std::string>(expr.literal));
			return;
		}
		throw std::runtime_error("Unreached code");
	}

	static void SetLiteralNormal(INode& node, int pin, const ExprContent& expr, const Script::VarType& type)
	{
		switch (expr.literal.index())
		{
		case 0: return;
		case 1:
		case 2:
			switch (type.type)
			{
			case Script::VarType::Bool:
				node.Set(pin, Enum{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Boolean);
				return;
			case Script::VarType::Guid:
				switch (std::any_cast<GuidEx>(type.extra))
				{
				case GuidEx::Entity:
					node.Set(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::GUID);
					break;
				case GuidEx::Prefab:
					node.Set(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Prefab);
					break;
				case GuidEx::Configuration:
					node.Set(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Configuration);
					break;
				case GuidEx::Faction:
					node.Set(pin, GUID{ (unsigned)expr.Get<int64_t>() }, ServerVarType::Faction);
					break;
				}
				return;
			case Script::VarType::Int:
				node.Set(pin, expr.Get<uint64_t>());
				return;
			case Script::VarType::Float:
				node.Set(pin, expr.Get<float>());
				return;
			default: throw std::runtime_error("Type mismatch");
			}
		case 3:
			node.Set(pin, std::get<std::string>(expr.literal));
			return;
		}
		throw std::runtime_error("Unreached code");
	}

	auto layout() { return [this](INode* n) { AutoLayout(n); }; }

	void VisitEvent(const std::string& event, const std::vector<Variable>& parameters) override
	{
		const EventProto& ep = *(proto = EventRegistry.Lookup(event, parameters));
		if (prev) { x = 0; y += 800; }
		prev = &ep.Create(graph);
		entrypoint = prev;
		AutoLayout(prev);
		flow = 0;
		for (auto& a : parameters)
		{
			unsigned pin = 0;
			for (auto& [id, type] : ep.parameters)
			{
				if (id == a.Id()) break;
				++pin;
			}
			scope.add(a.Id(), std::make_unique<LocalVar>(a.Type(), VarContent{ pin }));
		}
		function_header.in_function = false;
	}

	void VisitFunction(const std::string& name, std::optional<Script::VarType> ret, const std::vector<Variable>& parameters) override
	{
		if (function_storage.map.contains(name) || compiler.GlobalFunctions.map.contains(name)) throw std::runtime_error(std::format("function '{}' is already defined", name));
		function_header.in_function = true;
		if (prev) { x = 0; y += 800; }
		graph.AddComment(std::format("function {}", name), x - 400, y);
		flow = 0;
		auto& [dp, dr, de] = function_storage.map[name];
		for (auto& p : parameters)
		{
			auto n = &graph.AddNode(NodeFactory::GetLocalVariable(graph, p.Type()));
			AutoLayout(n);
			scope.add(p.Id(), std::make_unique<LocalVar>(p.Type(), VarContent{ n }));
			n->SetComment(p.Id());
			dp.emplace_back(n, p.Type());
		}
		if (ret.has_value())
		{
			auto n = function_header.ret = &graph.AddNode(NodeFactory::GetLocalVariable(graph, *ret));
			AutoLayout(n);
			n->SetComment("return");
			dr = { n,*ret };
			function_header.type = std::move(ret);
		}
		de = entrypoint = prev = &graph.AddNode(DoubleBranch);
		prev->Set(0, Enum{ 1 }, ServerVarType::Boolean);
		prev->SetComment("Dummy EntryPoint");
		AutoLayout(prev);
	}

	void VisitGlobalFunction(const std::string& name, std::optional<Script::VarType> ret, const std::vector<Variable>& parameters)
	{
		if (compiler.GlobalFunctions.map.contains(name)) throw std::runtime_error(std::format("function '{}' is already defined", name));
		function_header.in_function = true;
		if (prev) { x = 0; y += 800; }
		auto& [dp, dr, gr] = compiler.GlobalFunctions.map[name];
		uint32_t pin = 0;
		for (auto& p : parameters)
		{
			auto n = &graph.AddNode(NodeFactory::GetLocalVariable(graph, p.Type()));
			AutoLayout(n);
			scope.add(p.Id(), std::make_unique<LocalVar>(p.Type(), VarContent{ n }));
			n->SetComment(p.Id());
			auto& sn = graph.AddNode(NodeFactory::SetLocalVariable(graph, p.Type()));
			AutoLayout(&sn);
			n->Connect(sn, 0, 0);
			DefinePin(sn, p.Type(), 1, true, false);
			graph.SetCompositePin(sn, PinType::Input, 1, pin);
			graph.SetCompositePinName(PinType::Input, pin++, p.Id());
			if (!entrypoint) entrypoint = &sn;
			if (prev) prev->Connect(sn);
			prev = &sn;
			dp.emplace_back(p.Type());
		}
		if (ret.has_value())
		{
			auto n = function_header.ret = &graph.AddNode(NodeFactory::GetLocalVariable(graph, *ret));
			AutoLayout(n);
			n->SetComment("return");
			DefinePin(*n, *ret, 1, true, true);
			graph.SetCompositePin(*n, PinType::Output, 1, 0);
			dr = *ret;
			function_header.type = std::move(ret);
		}
		if (!entrypoint)
		{
			entrypoint = prev = &graph.AddNode(DoubleBranch);
			prev->Set(0, Enum{ 1 }, ServerVarType::Boolean);
			prev->SetComment("Dummy EntryPoint");
			AutoLayout(prev);
		}
		graph.SetCompositePin(*entrypoint, PinType::Inflow, 0, 0);
		gr = &graph;
	}

	Script::VarType TypeInference(const std::any& value) override
	{
		if (!value.has_value()) throw std::runtime_error("Variables defined by 'var' must be initialized");
		return std::any_cast<ExprContent*>(value)->retType;
	}

	void VisitVarDef(const std::string& id, Script::VarType type, const std::any& value) override
	{
		if (KEYWORDS.contains(id)) throw std::runtime_error(std::format("Cannot use keyword '{}' as identifier here", id));
		if (type.type == Script::VarType::Unknown) throw std::runtime_error("Unknown variable type");
		if (FunctionRegistry.Find(id).has_value() || function_storage.map.contains(id)) throw std::runtime_error(std::format("Identifier '{}' is already defined by a function", id));
		if (scope.contains(id)) throw std::runtime_error(std::format("Variable '{}' is already defined in current scope", id));
		auto n = &graph.AddNode(NodeFactory::GetLocalVariable(graph, type));
		AutoLayout(n);
		scope.add(id, std::make_unique<LocalVar>(type, VarContent{ n }));
		n->SetComment(id);
		if (value.has_value())
		{
			auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
			if (expr->retType.type == Script::VarType::Tuple) throw std::runtime_error("Cannot use tuple in here");
			if (expr->extra.index() == 3)
			{
				auto& il = std::get<3>(expr->extra);
				switch (type.type)
				{
				case Script::VarType::Vec:
				{
					if (il.size() > 3) throw std::runtime_error("Too many item in initializer list");
					Vec vec;
					bool flag = false;
					for (int i = 0; i < 3; i++)
					{
						if (auto v = i < il.size() ? il[i].get() : nullptr)
						{
							if (v->literal.index() == 0)
							{
								flag = true;
								break;
							}
							((float*)&vec)[i] = v->Get<float>();
						}
						else ((float*)&vec)[i] = 0;
					}
					if (flag)
					{
						auto& cr = graph.AddNode(Create3DVector);
						for (int i = 0; i < 3; i++)
						{
							if (auto v = i < il.size() ? il[i].get() : nullptr)
							{
								if (v->retType.type != Script::VarType::Float) throw std::runtime_error("Type mismatch with initializer list item");
								if (v->literal.index() == 0)
								{
									v->Add(graph, layout());
									v->end->Connect(cr, v->pin, i);
								}
								else cr.Set(i, v->Get<float>());
							}
							else cr.Set(i, 0.f);
						}
						AutoLayout(&cr);
						auto& n2 = graph.AddNode(NodeFactory::SetLocalVariable(graph, type));
						AutoLayout(&n2);
						prev->Connect(n2, flow, 0, true);
						n->Connect(n2, 0, 0);
						cr.Connect(n2, 0, 1);
						prev = &n2;
						flow = 0;
					}
					else n->Fill(0, vec);
					break;
				}
				case Script::VarType::List:
				{
					if (il.empty()) return;
					auto& as = graph.AddNode(AssemblyListInt);
					as.Set(0, il.size());
					unsigned pin = 1;
					for (auto& i : il)
					{
						if (i->retType != std::any_cast<Script::VarType&>(type.extra)) throw std::runtime_error("Type mismatch with initializer list item");
						if (i->literal.index() == 0)
						{
							i->Add(graph, layout());
							i->end->Connect(as, i->pin, pin);
						}
						else
						{
							switch (i->literal.index())
							{
							case 1:
								as.Set(pin, 0, (uint64_t)i->Get<int64_t>());
								break;
							case 2:
								as.Set(pin, 4, i->Get<float>());
								break;
							case 3:
								as.Set(pin, 1, std::get<std::string>(i->literal));
								break;
							default: throw std::runtime_error("Type mismatch with initializer list item value");
							}
						}
						++pin;
					}
					AutoLayout(&as);
					auto& n2 = graph.AddNode(NodeFactory::SetLocalVariable(graph, type));
					AutoLayout(&n2);
					prev->Connect(n2, flow, 0, true);
					n->Connect(n2, 0, 0);
					as.Connect(n2, 0, 1);
					prev = &n2;
					flow = 0;
					break;
				}
				case Script::VarType::Map:
					throw std::runtime_error("Cannot use map type for local variable");
				default: throw std::runtime_error("Cannot use initializer list for this type");
				}
				return;
			}
			if (expr->literal.index() == 0)
			{
				auto& n2 = graph.AddNode(NodeFactory::SetLocalVariable(graph, type));
				expr->Add(graph, layout());
				AutoLayout(&n2);
				prev->Connect(n2, flow, 0, true);
				n->Connect(n2, 0, 0);
				expr->end->Connect(n2, expr->pin, 1);
				prev = &n2;
				flow = 0;
				return;
			}
			SetLiteral(*n, 0, *expr, type);
		}
	}

	void VisitExprStatement(const std::any& value) override
	{
		if (auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value)); expr->flowStart)
		{
			expr->Add(graph, layout());
			prev->Connect(*expr->flowStart, flow, 0, true);
			for (auto br : expr->branches) if (auto n = graph.Find(br)) prev->Connect(*n);
			if (!expr->flowEnd) throw std::exception("Unknown error");
			prev = expr->flowEnd;
			flow = 0;
		}
	}

	void VisitIfStatement(IfStatement::Phase phase, std::any& value) override
	{
		struct IfContext
		{
			INode* branch;
			INode* merge;
		};
		switch (phase)
		{
		case IfStatement::Start:
		{
			IfContext ctx;
			auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
			if (expr->retType.type != Script::VarType::Bool) throw std::runtime_error("Condition expression must be boolean");
			auto& br = graph.AddNode(DoubleBranch);
			expr->Add(graph, layout());
			AutoLayout(&br);
			ctx.branch = &br;
			ctx.merge = prev;
			value = ctx;
			if (expr->flowStart)
			{
				prev->Connect(*expr->flowStart, flow, 0, true);
				prev = expr->flowEnd;
				flow = 0;
			}
			prev->Connect(br, flow, 0, true);
			if (expr->end) expr->end->Connect(br, expr->pin, 0);
			else br.Set(0, Enum{ (unsigned)std::get<int64_t>(expr->literal) }, ServerVarType::Boolean);
			prev = &br;
			flow = 0;
			return;
		}
		case IfStatement::Else:
		{
			auto& [branch, merge] = std::any_cast<IfContext&>(value);
			prev = branch;
			flow = 1;
			return;
		}
		case IfStatement::End:
		{
			auto& [branch, merge] = std::any_cast<IfContext&>(value);
			prev = merge;
			flow = 0;
		}
		}
	}

	struct SwitchContext
	{
		INode* branch;
		INode* merge;
		Script::VarType::Type type;
		int index;
		std::vector<std::variant<int, std::string>> values;
	};

	void VisitSwitchStatement(int count, std::any& value, bool end) override
	{
		if (end)
		{
			auto& ctx = std::any_cast<SwitchContext&>(value);
			if (ctx.type == Script::VarType::Int)
			{
				List<uint64_t> list;
				for (auto& v : ctx.values) list.emplace_back(std::get<0>(v));
				ctx.branch->Set(1, 0, list);
			}
			else
			{
				List<std::string> list;
				for (auto& v : ctx.values) list.emplace_back(std::get<1>(v));
				ctx.branch->Set(1, 1, list);
			}
			prev = ctx.merge;
			flow = 0;
			return;
		}
		SwitchContext ctx;
		auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		NodeId id;
		switch (expr->retType.type)
		{
		case Script::VarType::Int:
			id = MultipleBranchesInt;
			break;
		case Script::VarType::String:
			id = MultipleBranchesStr;
			break;
		default: throw std::runtime_error("Switch expression must be int or string");
		}
		auto& br = graph.AddNode(id);
		expr->Add(graph, layout());
		AutoLayout(&br);
		ctx.branch = &br;
		ctx.merge = prev;
		ctx.index = 0;
		ctx.type = expr->retType.type;
		value = ctx;
		if (expr->flowStart)
		{
			prev->Connect(*expr->flowStart, flow, 0, true);
			prev = expr->flowEnd;
			flow = 0;
		}
		prev->Connect(br, flow, 0, true);
		if (expr->end) expr->end->Connect(br, expr->pin, 0);
		else switch (expr->retType.type)
		{
		case Script::VarType::Int:
			br.Set(0, 0, (uint64_t)expr->Get<int64_t>());
			break;
		case Script::VarType::String:
			br.Set(0, 1, std::get<std::string>(expr->literal));
			break;
		default: throw std::exception("Unreached code");
		}
		prev = &br;
	}

	void VisitCase(const std::any& literal, std::any& value) override
	{
		if (literal.has_value())
		{
			auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(literal));
			auto& ctx = std::any_cast<SwitchContext&>(value);
			prev = ctx.branch;
			flow = ++ctx.index;
			if (expr->literal.index() == 0) throw std::runtime_error("Switch case value must be literal");
			switch (ctx.type)
			{
			case Script::VarType::Int:
				ctx.values.emplace_back((int)expr->Get<int64_t>());
				break;
			case Script::VarType::String:
				ctx.values.emplace_back(std::get<std::string>(expr->literal));
				break;
			default: throw std::exception("Unreached code");
			}
		}
		else
		{
			auto& ctx = std::any_cast<SwitchContext&>(value);
			prev = ctx.branch;
			flow = 0;
		}
	}

	struct LoopContext
	{
		INode* loop;
		INode* cond;
		INode* old;
	};

	void VisitWhile(std::any& value, bool end) override
	{
		if (end)
		{
			auto& [loop, cond, old] = std::any_cast<LoopContext&>(value);
			auto& exit = graph.AddNode(BreakLoop);
			AutoLayout(&exit);
			cond->Connect(exit, 1, 0, true);
			exit.Connect(*loop, 0, 1, true);
			prev = loop;
			current_loop = old;
			flow = 1;
			return;
		}
		auto& loop = graph.AddNode(FiniteLoop);
		AutoLayout(&loop);
		loop.Set(0, 0);
		loop.Set(1, INT_MAX);
		prev->Connect(loop, flow, 0, true);
		auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		if (expr->retType.type != Script::VarType::Bool) throw std::runtime_error("Condition expression must be boolean");
		auto& br = graph.AddNode(DoubleBranch);
		expr->Add(graph, layout());
		AutoLayout(&br);
		loop.Connect(br);
		if (expr->end) expr->end->Connect(br, expr->pin, 0);
		else br.Set(0, Enum{ (unsigned)std::get<int64_t>(expr->literal) }, ServerVarType::Boolean);
		value = LoopContext{ &loop,&br,current_loop };
		prev = &br;
		current_loop = &loop;
		flow = 0;
	}

	void VisitFor(std::any& value, bool end) override
	{
		if (end)
		{
			auto& [loop, cond, old] = std::any_cast<LoopContext&>(value);
			if (cond)
			{
				auto& exit = graph.AddNode(BreakLoop);
				AutoLayout(&exit);
				cond->Connect(exit, 1, 0, true);
				exit.Connect(*loop, 0, 1, true);
			}
			prev = loop;
			current_loop = old;
			flow = 1;
			return;
		}
		auto& loop = graph.AddNode(FiniteLoop);
		AutoLayout(&loop);
		loop.Set(0, 0);
		loop.Set(1, INT_MAX);
		prev->Connect(loop, flow, 0, true);
		if (value.has_value())
		{
			auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
			if (expr->retType.type != Script::VarType::Bool) throw std::runtime_error("Condition expression must be boolean");
			auto& br = graph.AddNode(DoubleBranch);
			expr->Add(graph, layout());
			AutoLayout(&br);
			if (expr->flowStart)
			{
				loop.Connect(*expr->flowStart);
				expr->flowEnd->Connect(br);
			}
			else loop.Connect(br);
			if (expr->end) expr->end->Connect(br, expr->pin, 0);
			else br.Set(0, Enum{ (unsigned)std::get<int64_t>(expr->literal) }, ServerVarType::Boolean);
			value = LoopContext{ &loop,&br,current_loop };
			prev = &br;
		}
		else
		{
			value = LoopContext{ &loop,nullptr,current_loop };
			prev = &loop;
		}
		current_loop = &loop;
		flow = 0;
	}

	void VisitForEachStart(Script::VarType type, const std::string& var, std::any& value) override
	{
		auto iterable = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		if (iterable->retType.type != Script::VarType::List) throw std::runtime_error("Expression is not iterable");
		if (type.type == Script::VarType::Unknown) type = std::any_cast<Script::VarType>(iterable->retType.extra);
		else if (type != std::any_cast<Script::VarType>(iterable->retType.extra)) throw std::runtime_error("Iterator type mismatch");
		auto& loop = graph.AddNode(ListIterationLoopInt);
		iterable->Add(graph, layout());
		AutoLayout(&loop);
		if (iterable->flowStart)
		{
			prev->Connect(*iterable->flowStart, flow, 0, true);
			prev = iterable->flowEnd;
			flow = 0;
		}
		prev->Connect(loop, flow, 0, true);
		iterable->end->Connect(loop, iterable->pin, 0);
		scope.add(var, std::make_unique<LocalVar>(type, VarContent{ &loop,true }));
		value = LoopContext{ &loop,nullptr,current_loop };
		prev = &loop;
		current_loop = &loop;
		flow = 0;
	}

	void VisitForEachEnd(std::any& value) override
	{
		auto& [loop, cond, old] = std::any_cast<LoopContext&>(value);
		prev = loop;
		current_loop = old;
		flow = 1;
	}

	void VisitBreak() override
	{
		if (!current_loop) throw std::runtime_error("Keyword 'break' is only used in the loop");
		auto& exit = graph.AddNode(BreakLoop);
		AutoLayout(&exit);
		prev->Connect(exit, flow, 0, true);
		exit.Connect(*current_loop, 0, 1, true);
	}

	std::any VisitLiteral(Literal::Type type, const std::any& value) override
	{
		switch (type)
		{
		case Literal::Int:
			return new ExprContent(std::any_cast<int64_t>(value));
		case Literal::Float:
			return new ExprContent(std::any_cast<float>(value));
		case Literal::Bool:
			return new ExprContent(std::any_cast<bool>(value));
		case Literal::String:
			return new ExprContent(std::any_cast<std::string>(value));
		case Literal::Null:
			return new ExprContent(0);
		default: throw std::runtime_error("Unsupported literal type");
		}
	}

	std::any VisitAssignment(const std::any& ref, Assignment::Op op, const std::any& value) override
	{
		auto ref_value = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(ref));
		if (ref_value->extra.index() != 1) throw std::runtime_error("Cannot assign to rvalue");
		auto& lvalue = std::get<LValueContext>(ref_value->extra);
		auto var_pin = lvalue.local ? 1 : 2;
		auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		if (expr->retType.type == Script::VarType::Tuple) throw std::runtime_error("Cannot use tuple in here");
		auto newExpr = std::make_unique<ExprContent>();
		ExprBuilder builder(*newExpr);
		newExpr->retType = ref_value->retType.type == Script::VarType::Unknown ? expr->retType : ref_value->retType;
		newExpr->extra = ref_value->extra;
		INode* opn = nullptr;
		if (!lvalue.local) NodeFactory::GetCustomVariable(ref_value->end, newExpr->retType);
		auto ret = ref_value->end;
		auto ret_pin = ref_value->pin;
		switch (op)
		{
		case Assignment::Add:
			opn = builder.Add(NodeFactory::Add(graph, *newExpr, *expr));
			builder.Combine(*ref_value, 0);
			break;
		case Assignment::Sub:
			opn = builder.Add(NodeFactory::Sub(graph, *newExpr, *expr));
			builder.Combine(*ref_value, 0);
			break;
		case Assignment::Mul:
			opn = builder.Add(NodeFactory::Mul(graph, *newExpr, *expr));
			builder.Combine(*ref_value, 0);
			break;
		case Assignment::Div:
			opn = builder.Add(NodeFactory::Div(graph, *newExpr, *expr));
			builder.Combine(*ref_value, 0);
			break;
		case Assignment::Normal:
		{
			auto n = builder.AddFlow(lvalue.CreateSetter(graph, newExpr->retType));
			builder.Combine(*ref_value, -1);
			if (expr->extra.index() == 3)
			{
				auto& il = std::get<3>(expr->extra);
				switch (newExpr->retType.type)
				{
				case Script::VarType::Vec:
				{
					if (il.size() > 3) throw std::runtime_error("Too many item in initializer list");
					Vec vec;
					bool flag = false;
					ExprContent tmp;
					ExprBuilder b2(tmp);
					for (int i = 0; i < 3; i++)
					{
						if (auto v = i < il.size() ? il[i].get() : nullptr)
						{
							if (v->literal.index() == 0)
							{
								flag = true;
								break;
							}
							((float*)&vec)[i] = v->Get<float>();
						}
						else ((float*)&vec)[i] = 0;
					}
					if (flag)
					{
						auto cr = b2.Add(graph.CreateNode(Create3DVector));
						for (int i = 0; i < 3; i++)
						{
							if (auto v = i < il.size() ? il[i].get() : nullptr)
							{
								if (v->retType.type != Script::VarType::Float) throw std::runtime_error("Type mismatch with initializer list item");
								if (v->literal.index() == 0)
								{
									tmp.end = cr;
									b2.Combine(*v, i);
								}
								else cr->Set(i, v->Get<float>());
							}
							else cr->Set(i, 0.f);
						}
						builder.Combine(tmp, var_pin);
					}
					else n->Fill(var_pin, vec);
					break;
				}
				case Script::VarType::List:
				{
					if (il.empty()) break;
					ExprContent tmp;
					ExprBuilder b2(tmp);
					auto as = b2.Add(graph.CreateNode(AssemblyListInt));
					as->Set(0, il.size());
					unsigned pin = 1;
					for (auto& i : il)
					{
						if (i->retType != std::any_cast<Script::VarType&>(newExpr->retType.extra)) throw std::runtime_error("Type mismatch with initializer list item");
						if (i->literal.index() == 0)
						{
							tmp.end = as;
							b2.Combine(*i, pin);
						}
						else
						{
							switch (i->literal.index())
							{
							case 1:
								as->Set(pin, 0, (uint64_t)i->Get<int64_t>());
								break;
							case 2:
								as->Set(pin, 4, i->Get<float>());
								break;
							case 3:
								as->Set(pin, 1, std::get<std::string>(i->literal));
								break;
							default: throw std::runtime_error("Type mismatch with initializer list item value");
							}
						}
						++pin;
					}
					builder.Combine(tmp, var_pin);
					break;
				}
				case Script::VarType::Map:
					throw std::runtime_error("Cannot use map type for local variable");
				default: throw std::runtime_error("Cannot use initializer list for this type");
				}
				return newExpr.release();
			}
			SetLiteral(*n, var_pin, *expr, newExpr->retType);
			newExpr->start = n;
			builder.Combine(*expr, var_pin);
			newExpr->end = ret;
			newExpr->pin = ret_pin;
			return newExpr.release();
		}
		}
		newExpr->start = opn;
		builder.Combine(*expr, 1);
		auto n = builder.AddFlow(lvalue.CreateSetter(graph, newExpr->retType));
		opn->Connect(*n, 0, var_pin);
		newExpr->end = ret;
		newExpr->pin = ret_pin;
		return newExpr.release();
	}

	std::any VisitCall(const std::any& value, const std::vector<std::any>& args, std::optional<Script::VarType> type) override
	{
		auto v = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		if (v->retType.type != Script::VarType::Function) throw std::runtime_error("Cannot call non-function");
		std::vector<Script::VarType> types;
		std::vector<std::unique_ptr<ExprContent>> exprs;
		for (const auto& a : args)
		{
			auto expr = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(a));
			types.push_back(expr->retType);
			exprs.push_back(std::move(expr));
		}
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		if (v->extra.index() == 4)
		{
			auto& uf = std::get<4>(v->extra);
			if (uf.global)
			{
				auto& func = compiler.GlobalFunctions.map[uf.id];
				if (exprs.size() != func.parameters.size()) throw std::runtime_error("Call parameters count not equal");
				auto call = builder.AddFlow(graph.CreateNode(*func.graph));
				unsigned i = 0;
				for (auto& arg : func.parameters)
				{
					auto j = i++;
					auto& e = exprs[j];
					if (e->retType != arg) throw std::runtime_error("Parameter type mismatch");
					if (e->literal.index() == 0) builder.Combine(*e, j);
					else SetLiteralNormal(*call, j, *e, arg);
					expr->start = call;
				}
				if (func.ret.has_value())
				{
					expr->retType = *func.ret;
					expr->end = call;
					expr->pin = 0;
				}
				return expr.release();
			}
			auto& func = function_storage.map[uf.id];
			if (exprs.size() != func.parameters.size()) throw std::runtime_error("Call parameters count not equal");
			unsigned i = 0;
			for (auto& arg : func.parameters)
			{
				auto& e = exprs[i++];
				if (e->retType != arg.type) throw std::runtime_error("Parameter type mismatch");
				auto n = builder.AddFlow(NodeFactory::SetLocalVariable(graph, arg.type));
				arg.node->Connect(*n, 0, 0);
				if (e->literal.index() == 0) builder.Combine(*e, 1);
				else SetLiteral(*n, 1, *e, arg.type);
			}
			if (func.ret.has_value())
			{
				expr->retType = func.ret.value().type;
				expr->end = func.ret.value().node;
				expr->pin = 1;
			}
			expr->flowEnd->Connect(*func.entrypoint);
			return expr.release();
		}
		auto& proto = FunctionRegistry::Lookup(std::get<2>(v->extra), types);
		auto call = proto.pure ? builder.Add(proto.Create(graph)) : builder.AddFlow(proto.Create(graph));
		unsigned pin = 0;
		for (auto& arg : exprs)
		{
			switch (arg->literal.index())
			{
			case 0:
				builder.Combine(*arg, pin);
				break;
			case 1:
				if (!proto.Contains(pin)) call->Set(pin, (uint64_t)arg->Get<int64_t>());
				else call->Fill(pin, arg->Get<int64_t>());
				break;
			case 2:
				if (!proto.Contains(pin)) call->Set(pin, arg->Get<float>());
				else call->Fill(pin, arg->Get<float>());
				break;
			case 3:
				if (!proto.Contains(pin)) call->Set(pin, std::get<std::string>(arg->literal));
				else call->Fill(pin, std::get<std::string>(arg->literal));
				break;
			case 4:
				if (!proto.Contains(pin)) call->Set(pin, Enum{ arg->Get<bool>() }, ServerVarType::Boolean);
				else call->Fill(pin, arg->Get<int64_t>());
				break;
			}
			pin++;
		}
		if (proto.ret.has_value())
		{
			expr->retType = *proto.ret;
			expr->end = call;
			expr->pin = 0;
		}
		return expr.release();
	}

	std::any VisitIdentifier(const std::string& id) override
	{
		if (auto func = FunctionRegistry.Find(id); func.has_value())
		{
			auto expr = std::make_unique<ExprContent>();
			expr->retType.type = Script::VarType::Function;
			expr->extra = *func;
			return expr.release();
		}
		if (auto func = compiler.GlobalFunctions.map.find(id); func != compiler.GlobalFunctions.map.end())
		{
			auto expr = std::make_unique<ExprContent>();
			expr->retType.type = Script::VarType::Function;
			expr->extra = UserFunction{ id ,true };
			return expr.release();
		}
		if (auto func = function_storage.map.find(id); func != function_storage.map.end())
		{
			auto expr = std::make_unique<ExprContent>();
			expr->retType.type = Script::VarType::Function;
			expr->extra = UserFunction{ id ,false };
			return expr.release();
		}
		auto var = scope.find(id);
		if (!var) throw std::runtime_error("Undefined symbol: " + id);
		auto& [content, iterator] = std::any_cast<VarContent&>(var->content);
		auto expr = std::make_unique<ExprContent>();
		if (std::holds_alternative<unsigned>(content))
		{
			auto pin = std::get<unsigned>(content);
			if (proto.has_value())
			{
				const EventProto& ep = *proto;
				unsigned i = 0;
				for (auto& [id, type] : ep.parameters)
				{
					if (i == pin)
					{
						expr->retType = type;
						break;
					}
					i++;
				}
				expr->end = entrypoint;
				expr->pin = pin;
			}
			else throw std::exception();
			return expr.release();
		}
		auto ref = std::get<INode*>(content);
		expr->end = ref;
		expr->retType = var->type;
		if (iterator)
		{
			expr->pin = 0;
			return expr.release();
		}
		expr->pin = 1;
		expr->extra = LValueContext{ var };
		return expr.release();
	}

	std::any VisitIncrement(const std::any& ref, bool inv, bool pre) override
	{
		if (pre) return VisitAssignment(ref, inv ? Assignment::Sub : Assignment::Add, new ExprContent(1));
		auto ref_value = std::any_cast<ExprContent*>(ref);
		if (ref_value->extra.index() != 1) throw std::runtime_error("Cannot assign to rvalue");
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		auto& lvalue = std::get<LValueContext>(ref_value->extra);
		expr->retType = ref_value->retType;
		expr->extra = ref_value->extra;
		if (!lvalue.local) NodeFactory::GetCustomVariable(ref_value->end, expr->retType);
		auto ret = ref_value->end;
		auto ret_pin = ref_value->pin;
		auto tmp = builder.Add(NodeFactory::GetLocalVariable(graph, expr->retType));
		auto n = builder.AddFlow(NodeFactory::SetLocalVariable(graph, expr->retType));
		builder.Combine(*ref_value, -1);
		tmp->Connect(*n, 0, 0);
		ret->Connect(*n, ret_pin, 1);
		ref_value->end = ret;
		ref_value->pin = ret_pin;
		auto e = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(VisitAssignment(ref, inv ? Assignment::Sub : Assignment::Add, new ExprContent(1))));
		std::swap(*expr, *e);
		builder.Combine(*e, 1);
		expr->end = tmp;
		expr->extra = {};
		return expr.release();
	}

	std::any VisitUnary(UnaryExpr::Op op, const std::any& value) override
	{
		auto v = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		INode* result = nullptr;
		switch (op)
		{
		case UnaryExpr::Negate:
			result = builder.Add(NodeFactory::Sub(graph, ExprContent(0), *v));
			expr->retType = v->retType;
			builder.Combine(*v, 1);
			break;
		case UnaryExpr::LogicalNOT:
			result = builder.Add(NodeFactory::Not(graph, *v));
			expr->retType = v->retType;
			break;
		case UnaryExpr::BitwiseNOT:
			result = builder.Add(NodeFactory::BitwiseNot(graph, *v));
			expr->retType = v->retType;
			break;
		}
		builder.Combine(*v, 0);
		expr->start = result;
		expr->pin = 0;
		return expr.release();
	}

	std::any VisitBinary(BinaryExpr::Op op, const std::any& l, const std::any& r) override
	{
		auto left = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(l));
		auto right = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(r));
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		INode* result = nullptr;
		switch (op)
		{
		case BinaryExpr::Add:
			result = builder.Add(NodeFactory::Add(graph, *left, *right));
			expr->retType = left->retType;
			break;
		case BinaryExpr::Sub:
			result = builder.Add(NodeFactory::Sub(graph, *left, *right));
			expr->retType = left->retType;
			break;
		case BinaryExpr::Mul:
			result = builder.Add(NodeFactory::Mul(graph, *left, *right));
			expr->retType = left->retType;
			break;
		case BinaryExpr::Div:
			result = builder.Add(NodeFactory::Div(graph, *left, *right));
			expr->retType = left->retType;
			break;
		case BinaryExpr::Mod:
			result = builder.Add(NodeFactory::Mod(graph, *left, *right));
			expr->retType = left->retType;
			break;
		case BinaryExpr::LT:
		case BinaryExpr::GT:
		case BinaryExpr::LE:
		case BinaryExpr::GE:
			result = builder.Add(NodeFactory::Compare(graph, *left, *right, op));
			expr->retType = { Script::VarType::Bool };
			break;
		case BinaryExpr::EQ:
			result = builder.Add(NodeFactory::Equal(graph, *left, *right));
			expr->retType = { Script::VarType::Bool };
			break;
		case BinaryExpr::NE:
			result = builder.Add(NodeFactory::Equal(graph, *left, *right));
			expr->retType = { Script::VarType::Bool };
			result->Connect(*builder.Add(NodeFactory::Not(graph, *expr)), 0, 0);
			break;
		case BinaryExpr::AND:
		case BinaryExpr::OR:
		case BinaryExpr::ShL:
		case BinaryExpr::ShR:
			result = builder.Add(NodeFactory::Bitwise(graph, *left, *right, op));
			expr->retType = left->retType;
			break;
		case BinaryExpr::ShA:
		{
			ExprContent dummy(Script::VarType{ Script::VarType::Int });
			auto sign = builder.Add(NodeFactory::Bitwise(graph, *left, ExprContent(31), BinaryExpr::ShR));
			auto fill = builder.Add(NodeFactory::Sub(graph, ExprContent(0), dummy));
			if (left->end) left->end->Connect(*sign, left->pin, 0);
			sign->Connect(*fill, 0, 1);
			auto n = builder.Add(NodeFactory::Sub(graph, ExprContent(32), *right));
			if (right->end)	right->end->Connect(*n, right->pin, 1);
			auto mask = builder.Add(NodeFactory::Bitwise(graph, dummy, dummy, BinaryExpr::ShL));
			fill->Connect(*mask, 0, 0);
			n->Connect(*mask, 0, 1);
			result = builder.Add(NodeFactory::Bitwise(graph, *left, *right, op));
			auto ret = builder.Add(NodeFactory::Bitwise(graph, dummy, dummy, BinaryExpr::OR));
			result->Connect(*ret, 0, 0);
			mask->Connect(*ret, 0, 1);
			expr->retType = left->retType;
			expr->start = result;
			break;
		}
		case BinaryExpr::LogAND:
		case BinaryExpr::LogOR:
			result = builder.Add(NodeFactory::Logical(graph, *left, *right, op));
			expr->retType = { Script::VarType::Bool };
			break;
		case BinaryExpr::XOR:
		case BinaryExpr::LogXOR:
			if (left->retType.type == Script::VarType::Bool) result = builder.Add(NodeFactory::Logical(graph, *left, *right, op));
			else result = builder.Add(NodeFactory::Bitwise(graph, *left, *right, op));
			expr->retType = left->retType;
			break;
		}
		builder.Combine(*left, 0);
		expr->start = result;
		builder.Combine(*right, 1);
		expr->pin = 0;
		return expr.release();
	}

	std::any VisitTernary(const std::any& e1, const std::any& e2, const std::any& e3) override
	{
		auto cond = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(e1));
		auto then = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(e2));
		auto other = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(e3));
		if (then->retType != other->retType) throw std::runtime_error("Type mismatch in conditional expression");
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		auto tmp = builder.Add(NodeFactory::GetLocalVariable(graph, then->retType));
		auto br = expr->start = builder.AddFlow(graph.CreateNode(DoubleBranch));
		builder.Combine(*cond, 0);
		auto set = expr->start = builder.AddFlow(NodeFactory::SetLocalVariable(graph, then->retType));
		SetLiteral(*set, 1, *then, then->retType);
		builder.Combine(*then, 1);
		tmp->Connect(*set, 0, 0);
		expr->flowEnd = br;
		set = expr->start = builder.AddFlow(NodeFactory::SetLocalVariable(graph, other->retType), 1);
		SetLiteral(*set, 1, *other, other->retType);
		builder.Combine(*other, 1);
		tmp->Connect(*set, 0, 0);
		expr->end = tmp;
		expr->retType = then->retType;
		expr->pin = 1;
		expr->flowEnd = nullptr;
		expr->branch = true;
		return expr.release();
	}

	std::any VisitCast(Script::VarType type, const std::any& value) override
	{
		auto v = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		if (v->retType == type) return v.release();
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		builder.Add(NodeFactory::Cast(graph, *v, type));
		builder.Combine(*v, 0);
		expr->retType = type;
		return expr.release();
	}

	std::any VisitMemberAccess(const std::any& value, const std::any& member, std::optional<Script::VarType> type) override
	{
		auto v = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		auto m = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(member));
		auto expr = std::make_unique<ExprContent>();
		ExprBuilder builder(*expr);
		switch (v->retType.type)
		{
		case Script::VarType::Entity:
		{
			if (m->retType.type != Script::VarType::String) throw std::runtime_error("Member not defined");
			if (type.has_value()) expr->retType = *type;
			auto n = builder.Add(NodeFactory::GetCustomVariable(graph, expr->retType));
			if (m->literal.index() != 0)
			{
				n->Set(1, std::get<std::string>(m->literal));
				expr->extra = LValueContext{ nullptr,v->end,std::get<std::string>(m->literal),v->pin };
			}
			else expr->extra = LValueContext{ nullptr,v->end,m->end,v->pin,m->pin };
			builder.Combine(*v, 0);
			builder.Combine(*m, 1);
			break;
		}
		case Script::VarType::Vec:
		{
			if (m->literal.index() != 3) throw std::runtime_error("Member not defined");
			auto id = std::get<std::string>(m->literal);
			builder.Add(graph.CreateNode(Split3DVector));
			if (id == "x") expr->pin = 0;
			else if (id == "y") expr->pin = 1;
			else if (id == "z") expr->pin = 2;
			else throw std::runtime_error("Member not defined");
			expr->retType.type = Script::VarType::Float;
			builder.Combine(*v, 0);
			break;
		}
		case Script::VarType::List:
		{
			if (m->retType.type != Script::VarType::Int) throw std::runtime_error("Member not defined");
			auto n = builder.Add(graph.CreateNode(GetCorrespondingValueFromListInt));
			builder.Combine(*v, 0);
			builder.Combine(*m, 1);
			if (m->literal.index() != 0) n->Set(1, (uint64_t)m->Get<int64_t>());
			expr->retType = std::any_cast<Script::VarType&>(v->retType.extra);
			break;
		}
		case Script::VarType::Map:
		case Script::VarType::Tuple:
			throw std::exception("Unimplemented");
		default: throw std::runtime_error("Type haven't member access operation");
		}
		return expr.release();
	}

	std::any VisitConstruct(Script::VarType type, const std::vector<std::any>& args) override
	{
		throw std::exception("Unimplemented");
	}

	std::any VisitInitializerList(const std::vector<std::any>& values) override
	{
		auto list = std::make_unique<ExprContent>();
		std::vector<std::shared_ptr<ExprContent>> eps;
		for (auto& v : values) eps.emplace_back(std::shared_ptr<ExprContent>(std::any_cast<ExprContent*>(v)));
		list->extra = std::move(eps);
		return list.release();
	}

	void VisitReturn(const std::any& value) override
	{
		if (!value.has_value()) return;
		auto v = std::unique_ptr<ExprContent>(std::any_cast<ExprContent*>(value));
		if (v->retType != function_header.type) throw std::runtime_error("Return type mismatch");
		auto r = function_header.ret;
		auto& n = graph.AddNode(NodeFactory::SetLocalVariable(graph, v->retType));
		v->Add(graph, layout());
		AutoLayout(&n);
		r->Connect(n, 0, 0);
		v->end->Connect(n, v->pin, 1);
		if (v->flowStart)
		{
			prev->Connect(*v->flowStart, flow, 0, true);
			prev = v->flowEnd;
		}
		prev->Connect(n);
		prev = &n;
	}
};

void Compiler::AddGlobalFunction(const std::string& name, std::unique_ptr<FunctionNode> func)
{
	auto& [graph, ast] = symbol_modules.emplace_back(CreateGraph(name, GraphType::Composite), std::move(func));
	project->Define(*graph);
}

Compiler::Compiler(std::unique_ptr<IProject> project) : project(std::move(project))
{
}

void Compiler::AddModule(const std::string& name, const std::string& code)
{
	auto& [graph, ast] = modules.emplace_back(CreateGraph(name, GraphType::Entity), Parse(code));
	project->Define(*graph);
	for (auto gfs = ((RootNode*)ast.get())->GlobalFunctions(); auto& f : gfs)
	{
		auto name = f->Name();
		AddGlobalFunction("GIScript#" + name, std::move(f));
	}
}

void Compiler::Compile()
{
	std::vector<std::function<void()>> actions;
	for (auto& [graph, ast] : symbol_modules)
	{
		auto& f = *(FunctionNode*)ast.get();
		auto g = std::make_shared<NodeGenerator>(*graph, *this);
		g->scope.enter();
		g->VisitGlobalFunction(f.Name(), f.Ret(), f.Parameters());
		actions.emplace_back([&f, g, graph = graph.get()] mutable
			{
				f.VisitBody(*g);
				g->scope.exit();
				auto ex = g->prev;
				if (g->flow != 0)
				{
					ex = &graph->AddNode(NodeId::DoubleBranch);
					ex->Set(0, Enum{ 1 }, ServerVarType::Boolean);
					ex->SetComment("Dummy ExitPoint");
					g->AutoLayout(ex);
				}
				graph->SetCompositePin(*ex, PinType::Outflow, 0, 0);
			});
	}
	for (auto& a : actions) a();
	for (auto& [graph, ast] : modules)
	{
		NodeGenerator g(*graph, *this);
		ast->Visit(g);
	}
}

void Compiler::Write() const
{
	for (const auto& [graph, ast] : symbol_modules) project->Add(*graph);
	for (const auto& [graph, ast] : modules) project->Add(*graph);
}
