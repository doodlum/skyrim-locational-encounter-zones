#include "DynamicEncounterZones.h"
#include <detours/Detours.h>

decltype(&DynamicEncounterZones::hk_GetEncounterZone) ptrGetEncounterZone;

void DynamicEncounterZones::InstallHooks()
{
	logger::info("Detouring virtual function tables");
	*(uintptr_t*)&ptrGetEncounterZone = Detours::X64::DetourFunction(REL::RelocationID(19797, 20202).address(), (uintptr_t)&hk_GetEncounterZone);
}

RE::BGSEncounterZone* DynamicEncounterZones::hk_GetEncounterZone(RE::TESObjectREFR* This)
{
	auto ret = ptrGetEncounterZone(This);
	if (!ret) {
		return GetSingleton()->GetEncounterZone(This);
	}
	return ret;
}

void DynamicEncounterZones::CacheEncounterZones()
{
	auto  dataHandler = RE::TESDataHandler::GetSingleton();
	auto& encounterZoneArray = dataHandler->GetFormArray<RE::BGSEncounterZone>();
	for (auto& encounterZone : encounterZoneArray)
	{
		logger::info("Looking at encounter zone {:X}", encounterZone->GetFormID());
		if (auto location = encounterZone->data.location)
		{
			if (!encounterZone->data.flags.all(RE::ENCOUNTER_ZONE_DATA::Flag::kNeverResets))
			{
				locationToEncounterZoneMap.insert({ location, encounterZone });
			}
			else {
				logger::info("Had never resets flag, ignoring");
			}
		}
		else {
			logger::info("Had no linked location, ignoring");
		}
	}
}

RE::BGSEncounterZone* DynamicEncounterZones::GetEncounterZone(RE::TESObjectREFR* This)
{
	std::lock_guard<std::shared_mutex> lk(dataLock);
	if (!dataLoaded)
	{
		CacheEncounterZones();
		dataLoaded = true;
	}

	if (This->As<RE::Actor>())
	{
		if (auto name = This->GetName())
		{
			logger::info("Patching Actor {} ({:X})", name, This->GetFormID());
		}
		else {
			logger::info("Patching Actor {:X}", This->GetFormID());
		}

		if (auto location = This->GetCurrentLocation())
		{
			if (auto name = location->GetName())
			{
				logger::info("Has location {}", name);
			}
			else {
				logger::info("Has location (unnamed)");
			}
			auto it = locationToEncounterZoneMap.find(location);
			if (it != locationToEncounterZoneMap.end())
			{
				logger::info("Found encounter zone for location, returning");
				return (*it).second;
			}
			else {
				logger::info("Could not find encounter zone for location");
			}
		}
		else {
			logger::info("Has no location");
		}
		logger::info("Returning nothing");
	}
	return nullptr;
}