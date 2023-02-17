#include "DynamicEncounterZones.h"
#include <detours/Detours.h>

decltype(&DynamicEncounterZones::hk_GetEncounterZone) ptrGetEncounterZone;

typedef RE::BGSEncounterZone* (*GetEncounterZone_t)(RE::TESObjectREFR* This);

void DynamicEncounterZones::InstallHooks()
{
	logger::info("Installing TESObjectREFR::GetEncounterZone function hook\n\n");
	*(uintptr_t*)&ptrGetEncounterZone = Detours::X64::DetourFunction(REL::RelocationID(19797, 20202).address(), (uintptr_t)&hk_GetEncounterZone);
}

RE::BGSEncounterZone* DynamicEncounterZones::hk_GetEncounterZone(RE::TESObjectREFR* This)
{
	auto ret = (ptrGetEncounterZone)(This);
	if (!ret)
	{
		return GetSingleton()->GetEncounterZone(This);
	}
	return ret;
}

void DynamicEncounterZones::CacheEncounterZones()
{
	logger::info("Caching encounter zone to location pairs\n\n");
	std::lock_guard<std::shared_mutex> lk(dataLock);
	auto  dataHandler = RE::TESDataHandler::GetSingleton();
	auto& encounterZoneArray = dataHandler->GetFormArray<RE::BGSEncounterZone>();
	for (auto& encounterZone : encounterZoneArray)
	{
		logger::info("	Looking at encounter zone {:X}", encounterZone->GetFormID());
		if (auto location = encounterZone->data.location)
		{
			if (auto name = location->GetName())
			{
				logger::info("		Has location {}", name);
			}
			else {
				logger::info("		Has location (unnamed)");
			}
			auto it = locationToEncounterZoneMap.find(location);
			if (it != locationToEncounterZoneMap.end()) {
				auto conflictingEncounterZone = (*it).second;
				logger::warn("			Encounter zone {:X} conflicts with {:X}", encounterZone->GetFormID(), conflictingEncounterZone->GetFormID());
				if (max(encounterZone->data.minLevel, encounterZone->data.maxLevel) > max(conflictingEncounterZone->data.minLevel, conflictingEncounterZone->data.maxLevel))
				{
					logger::info("				Replacing encounter zone due to higher level");
					locationToEncounterZoneMap.insert({ location, encounterZone });
				}
			}
			else {
				logger::info("			Inserting encounter zone for location");
				locationToEncounterZoneMap.insert({ location, encounterZone });
			}
		}
		else {
			logger::info("	Had no linked location, ignoring");
		}
	}
	logger::info("Finished caching\n\n");
	dataLoaded = true;
}

RE::BGSEncounterZone* DynamicEncounterZones::GetEncounterZone(RE::TESObjectREFR* This)
{
	std::lock_guard<std::shared_mutex> lk(dataLock);
	if (dataLoaded)
	{
		if (This->As<RE::Actor>())
		{
			if (auto name = This->GetName())
			{
				logger::info("\nActor {} ({:X})", name, This->GetFormID());
			}
			else {
				logger::info("\nActor {:X}", This->GetFormID());
			}

			if (auto location = This->GetCurrentLocation())
			{
				if (auto name = location->GetName())
				{
					logger::info("	Has location {}", name);
				}
				else {
					logger::info("	Has location (unnamed)");
				}
				auto it = locationToEncounterZoneMap.find(location);
				if (it != locationToEncounterZoneMap.end())
				{
					logger::info("		Found encounter zone for location, returning");
					return (*it).second;
				}
				else {
					logger::info("		Could not find encounter zone for location");
				}
			}
			else {
				logger::info("	Has no location");
			}
			logger::info("Returning nothing");
		}
	}
	return nullptr;
}