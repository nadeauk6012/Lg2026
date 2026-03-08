/*
 * Copyright (C) 2008-2026 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

#include "Config.h"
#include "Log.h"

namespace
{
    boost::property_tree::ptree _config;
    std::string _filename;
    std::mutex _configLock;

    // removes quotes from ini values: "foo" - foo
    inline void StripQuotes(std::string& s)
    {
        s.erase(std::remove(s.begin(), s.end(), '"'), s.end());
    }
}

namespace bpt = boost::property_tree;

bool ConfigMgr::LoadInitial(std::string const& file, std::string& error)
{
    std::lock_guard<std::mutex> lock(_configLock);

    _filename = file;

    try
    {
        bpt::ptree fullTree;
        read_ini(file, fullTree);

        if (fullTree.empty())
        {
            error = "empty file (" + file + ")";
            return false;
        }

        // Since we're using only one section per config file, we skip the section and have direct property access
        _config = fullTree.begin()->second;
    }
    catch (bpt::ini_parser::ini_parser_error const& e)
    {
        if (e.line() == 0)
            error = e.message() + " (" + e.filename() + ")";
        else
            error = e.message() + " (" + e.filename() + ":" + std::to_string(e.line()) + ")";
        return false;
    }

    return true;
}

ConfigMgr* ConfigMgr::instance()
{
    static ConfigMgr instance;
    return &instance;
}

bool ConfigMgr::Reload(std::string& error)
{
    return LoadInitial(_filename, error);
}

std::string ConfigMgr::GetStringDefault(std::string const& name, std::string const& def)
{
    try
    {
        std::string value = _config.get<std::string>(bpt::ptree::path_type(name, '/'));
        StripQuotes(value);
        return value;
    }
    catch (bpt::ptree_bad_path const&)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Missing name %s in config file %s, add \"%s = %s\" to this file",
            name.c_str(), _filename.c_str(), name.c_str(), def.c_str());
        return def;
    }
    catch (bpt::ptree_bad_data const& e)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Invalid value for %s in config file %s (%s). Using default \"%s\"",
            name.c_str(), _filename.c_str(), e.what(), def.c_str());
        return def;
    }
}

bool ConfigMgr::GetBoolDefault(std::string const& name, bool def)
{
    try
    {
        std::string val = _config.get<std::string>(bpt::ptree::path_type(name, '/'));
        StripQuotes(val);

        if (val == "true" || val == "TRUE" || val == "yes" || val == "YES" || val == "1")
            return true;

        if (val == "false" || val == "FALSE" || val == "no" || val == "NO" || val == "0")
            return false;

        // Key exists but is not a valid boolean token
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Invalid boolean value for %s in config file %s (\"%s\"). Using default %d",
            name.c_str(), _filename.c_str(), val.c_str(), def ? 1 : 0);
        return def;
    }
    catch (bpt::ptree_bad_path const&)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Missing name %s in config file %s, add \"%s = %d\" to this file",
            name.c_str(), _filename.c_str(), name.c_str(), def ? 1 : 0);
        return def;
    }
    catch (bpt::ptree_bad_data const& e)
    {

        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Invalid value for %s in config file %s (%s). Using default %d",
            name.c_str(), _filename.c_str(), e.what(), def ? 1 : 0);
        return def;
    }
}

int ConfigMgr::GetIntDefault(std::string const& name, int def)
{
    try
    {
        return _config.get<int>(bpt::ptree::path_type(name, '/'));
    }
    catch (bpt::ptree_bad_path const&)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Missing name %s in config file %s, add \"%s = %d\" to this file",
            name.c_str(), _filename.c_str(), name.c_str(), def);
        return def;
    }
    catch (bpt::ptree_bad_data const& e)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Invalid value for %s in config file %s (%s). Using default %d",
            name.c_str(), _filename.c_str(), e.what(), def);
        return def;
    }
}

float ConfigMgr::GetFloatDefault(std::string const& name, float def)
{
    try
    {
        return _config.get<float>(bpt::ptree::path_type(name, '/'));
    }
    catch (bpt::ptree_bad_path const&)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Missing name %s in config file %s, add \"%s = %f\" to this file",
            name.c_str(), _filename.c_str(), name.c_str(), def);
        return def;
    }
    catch (bpt::ptree_bad_data const& e)
    {
        TC_LOG_WARN(LOG_FILTER_SERVER_LOADING,
            "Invalid value for %s in config file %s (%s). Using default %f",
            name.c_str(), _filename.c_str(), e.what(), def);
        return def;
    }
}

std::string const& ConfigMgr::GetFilename()
{
    std::lock_guard<std::mutex> lock(_configLock);
    return _filename;
}

std::vector<std::string> ConfigMgr::GetKeysByString(std::string const& name)
{
    std::lock_guard<std::mutex> lock(_configLock);

    std::vector<std::string> keys;
    for (auto const& child : _config)
        if (child.first.compare(0, name.length(), name) == 0)
            keys.push_back(child.first);

    return keys;
}

