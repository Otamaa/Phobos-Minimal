#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <algorithm>

#include <Lib/inipp/inipp.h>

using IniStructure = inipp::Ini<char>;

std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> split_parents(const std::string& str)
{
	std::vector<std::string> tokens;
	std::istringstream ss(str);
	std::string token;
	while (std::getline(ss, token, ','))
	{
		token = trim(token);
		if (!token.empty()) tokens.push_back(token);
	}
	return tokens;
}

class AdvancedModIniParser
{
public:
	IniStructure ini;
	std::set<std::string> loaded_files;

	void load_file_recursive(const std::string& filename)
	{
		if (loaded_files.count(filename)) return;
		loaded_files.insert(filename);

		std::ifstream is(filename);
		if (!is.is_open()) return;

		// --- PRE-PROCESSING TRICK (PLUG & MODIFY) ---
		std::stringstream processed_stream;
		std::string line;

		while (std::getline(is, line))
		{
			std::string trimmed = trim(line);

			if (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']')
			{
				size_t colon_pos = trimmed.find(':');
				if (colon_pos != std::string::npos)
				{
					std::string child_part = trimmed.substr(1, colon_pos - 1);
					std::string parent_part = trimmed.substr(colon_pos + 1, trimmed.length() - colon_pos - 2);

					child_part = trim(child_part);
					parent_part = trim(parent_part);

					processed_stream << "[" << child_part << "]\n";
					processed_stream << "$Inherits=" << parent_part << "\n";
					continue;
				}
			}
			processed_stream << line << "\n";
		}
		// ----------------------------------------------

		IniStructure current_ini;
		current_ini.parse(processed_stream);

		std::vector<std::string> files_to_include;
		for (const std::string& include_sec : { "#include", "$Include" })
		{
			auto sec_it = current_ini.sections.find(include_sec);
			if (sec_it != current_ini.sections.end())
			{
				for (const auto& [key, val] : sec_it->second)
				{
					files_to_include.push_back(val);
				}
				current_ini.sections.erase(sec_it);
			}
		}

		for (auto& [sec_name, keys] : current_ini.sections)
		{
			std::vector<std::string> append_values;

			auto plus_it = keys.find("+");
			if (plus_it != keys.end())
			{
				append_values.push_back(plus_it->second);
				keys.erase(plus_it);
			}

			for (const auto& [key, val] : keys)
			{
				if (key == "$Inherits" && ini.sections[sec_name].count("$Inherits"))
				{
					ini.sections[sec_name][key] += "," + val;
				}
				else
				{
					ini.sections[sec_name][key] = val;
				}
			}

			for (const auto& val : append_values)
			{
				int index = 0;
				while (ini.sections[sec_name].count(std::to_string(index)))
				{
					index++;
				}
				ini.sections[sec_name][std::to_string(index)] = val;
			}
		}

		for (const std::string& next_file : files_to_include)
		{
			load_file_recursive(next_file);
		}
	}

	std::string get_value_internal(const std::string& section, const std::string& key, std::set<std::string>& visited)
	{
		if (visited.count(section)) return "";
		visited.insert(section);

		auto sec_it = ini.sections.find(section);
		if (sec_it != ini.sections.end())
		{
			auto key_it = sec_it->second.find(key);
			if (key_it != sec_it->second.end())
			{
				return key_it->second;
			}
		}

		if (sec_it != ini.sections.end())
		{
			auto inherit_it = sec_it->second.find("$Inherits");
			if (inherit_it != sec_it->second.end())
			{
				auto parents = split_parents(inherit_it->second);
				for (const auto& parent : parents)
				{
					std::string res = get_value_internal(parent, key, visited);
					if (!res.empty()) return res;
				}
			}
		}
		return "";
	}

	std::string getValue(const std::string& section, const std::string& key)
	{
		std::set<std::string> visited;
		return get_value_internal(section, key, visited);
	}
};

int main()
{
	// Simulasi file aturan rulesmd.ini dengan gaya pencabangan klasik C&C [Child]:[Parent]
	std::ofstream("rulesmd.ini") <<
		"[E1]\n"
		"Strength = 125\n"
		"Armor = None\n\n"

		"; Format Titik Dua (Classic C&C Style Inheritance)\n"
		"[E2] : [E1]\n"
		"Strength = 200\n" // Menimpa Strength milik E1
		"Weapon = M60\n";  // Menambahkan senjata baru

	AdvancedModIniParser parser;
	parser.load_file_recursive("rulesmd.ini");

	std::cout << "--- Hasil Cetakan Warisan Klasik [E2] ---\n";
	std::cout << "Weapon   : " << parser.getValue("E2", "Weapon") << " (Asli milik E2)\n";
	std::cout << "Strength : " << parser.getValue("E2", "Strength") << " (Override milik E1)\n";
	std::cout << "Armor    : " << parser.getValue("E2", "Armor") << " (Warisan otomatis dari E1)\n";

	return 0;
}