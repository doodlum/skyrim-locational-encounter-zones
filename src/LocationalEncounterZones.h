#pragma once

#include <shared_mutex>

constexpr auto steamAppID = "72850";

class LocationalEncounterZones
{
public:
	static LocationalEncounterZones* GetSingleton()
	{
		static LocationalEncounterZones singleton;
		return &singleton;
	}

	static void InstallHooks();

	static RE::BGSEncounterZone* hk_GetEncounterZone(RE::TESObjectREFR* This);

	std::shared_mutex dataLock;
	bool dataLoaded = false;
	std::map<void*, RE::BGSEncounterZone*> locationToEncounterZoneMap;
	std::set<RE::FormID> loggedForms;

	void CacheEncounterZones();
	RE::BGSEncounterZone* GetEncounterZone(RE::TESObjectREFR* This);

private:
	LocationalEncounterZones()
	{
	}

	LocationalEncounterZones(const LocationalEncounterZones&) = delete;
	LocationalEncounterZones(LocationalEncounterZones&&) = delete;

	~LocationalEncounterZones() = default;

	LocationalEncounterZones& operator=(const LocationalEncounterZones&) = delete;
	LocationalEncounterZones& operator=(LocationalEncounterZones&&) = delete;
};
