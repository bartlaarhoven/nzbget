/*
 *  This file is part of nzbget. See <https://nzbget.com>.
 *
 *  Copyright (C) 2023 Denis <denis@nzbget.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nzbget.h"
#include <fstream>
#include "ManifestFile.h"
#include "Json.h"
#include "FileSystem.h"

namespace ManifestFile
{
	const char* MANIFEST_FILE = "manifest.json";

	bool Load(Manifest& manifest, const char* directory)
	{
		BString<1024> path("%s%c%s", directory, PATH_SEPARATOR, MANIFEST_FILE);
		std::fstream fs(path);
		if (!fs.is_open())
			return false;

		Json::error_code ec;
		Json::object json = Json::Read(fs, ec).as_object();
		if (ec)
			return false;

		if (!ValidateRequiredAndSet(json, manifest))
			return false;

		CheckKeyAndSet(json, "task", manifest.task);
		ValidateOptionsAndSet(json, manifest.options);
		ValidateCommandsAndSet(json, manifest.commands);
		return true;
	}

	bool ValidateRequiredAndSet(Json::object& json, Manifest& manifest)
	{

		if (!CheckKeyAndSet(json, "author", manifest.author))
			return false;

		if (!CheckKeyAndSet(json, "entry", manifest.entry))
			return false;

		if (!CheckKeyAndSet(json, "description", manifest.description))
			return false;

		if (!CheckKeyAndSet(json, "version", manifest.version))
			return false;

		if (!CheckKeyAndSet(json, "name", manifest.name))
			return false;

		if (!CheckKeyAndSet(json, "displayName", manifest.displayName))
			return false;

		if (!CheckKeyAndSet(json, "kind", manifest.kind))
			return false;

		if (!CheckKeyAndSet(json, "license", manifest.license))
			return false;

		return true;
	}

	bool ValidateCommandsAndSet(Json::object& json, std::vector<Command>& commands)
	{
		auto rawCommands = json.if_contains("commands");
		if (!rawCommands)
			return false;

		for (auto& value : rawCommands->as_array())
		{
			Json::object cmdJson = value.as_object();
			Command command;

			if (!CheckKeyAndSet(cmdJson, "name", command.name))
				continue;

			if (!CheckKeyAndSet(cmdJson, "displayName", command.displayName))
				continue;

			if (!CheckKeyAndSet(cmdJson, "description", command.description))
				continue;

			if (!CheckKeyAndSet(cmdJson, "action", command.action))
				continue;

			commands.push_back(std::move(command));
		}

		return true;
	}

	bool ValidateOptionsAndSet(Json::object& json, std::vector<Option>& options)
	{
		auto rawOptions = json.if_contains("options");
		if (!rawOptions)
			return false;

		for (auto& value : rawOptions->as_array())
		{
			Json::object optionJson = value.as_object();
			auto selectJson = optionJson.if_contains("select");
			if (!selectJson)
				continue;

			Option option;

			if (!CheckKeyAndSet(optionJson, "name", option.name))
				continue;

			if (!CheckKeyAndSet(optionJson, "displayName", option.displayName))
				continue;

			if (!CheckKeyAndSet(optionJson, "description", option.description))
				continue;

			if (!CheckKeyAndSet(optionJson, "value", option.value))
				continue;

			for (auto& value : selectJson->as_array())
			{
				option.select.push_back(value.as_string().c_str());
			}

			options.push_back(std::move(option));
		}

		return true;
	}

	bool CheckKeyAndSet(Json::object& json, const char* key, std::string& property)
	{
		const auto& rawProperty = json.if_contains(key);
		if (!rawProperty)
			return false;

		property = rawProperty->as_string().c_str();
		return true;
	}
}