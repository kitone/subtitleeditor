/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "extensionmanager.h"
#include <glibmm/keyfile.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <iostream>
#include <vector>
#include "error.h"
#include "utility.h"
#include "cfg.h"

/*
 * Return the ExtensionManager instance.
 */
ExtensionManager& ExtensionManager::instance()
{
	static ExtensionManager instance;
	return instance;
}


/*
 * Constructor
 */
ExtensionManager::ExtensionManager()
{
	se_debug(SE_DEBUG_APP);

	// Read the user plugins
	load_path(get_config_dir("plugins"), false);

	// Read the env var if is set or the default plugin dir
	Glib::ustring path = Glib::getenv("SE_PLUGINS_PATH");
	if(path.empty())
		path = SE_DEV_VALUE(PACKAGE_PLUGIN_DESCRIPTION_DIR, PACKAGE_PLUGIN_DIR_DEV);
	
	load_path(path, true);
}

/*
 * Destructor
 */
ExtensionManager::~ExtensionManager()
{
	se_debug(SE_DEBUG_APP);

	destroy_extensions();
}

/*
 * Active and create extensions
 */
void ExtensionManager::create_extensions()
{
	se_debug(SE_DEBUG_APP);

	std::list<ExtensionInfo*> list = get_extension_info_list();
	for(std::list<ExtensionInfo*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		Glib::ustring state;
		if(Config::getInstance().get_value_string("extension-manager", (*it)->get_name(), state))
		{
			if(state == "enable")
				activate(*it);
		}
		else
		{
			// Unknow extension, enable by default
			se_debug_message(SE_DEBUG_APP, "First time for the plugin '%s', enable by default", (*it)->get_name().c_str());

			set_extension_active((*it)->get_name(), true);
		}
	}
}

/*
 * Delete and close all extensions
 */
void ExtensionManager::destroy_extensions()
{
	se_debug(SE_DEBUG_APP);

	std::list<ExtensionInfo*> list = get_extension_info_list();
	for(std::list<ExtensionInfo*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		se_debug_message(SE_DEBUG_APP, "delete extension '%s'", (*it)->get_name().c_str());
		delete *it;
	}
	
	m_extension_info_map.clear();
}

/*
 * Load the path and sub path to find extension description.
 * se-plugin file.
 */
void ExtensionManager::load_path(const Glib::ustring &path, bool fhs_directory)
{
	se_debug_message(SE_DEBUG_APP, "path=%s", path.c_str());

	if(Glib::file_test(path, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR) == false)
	{
		se_debug_message(SE_DEBUG_APP, "could not open the path %s", path.c_str());
		return;
	}

	try
	{
		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create("^(.*)\\.se-plugin$");
		
		Glib::Dir dir(path);

		std::vector<Glib::ustring> files(dir.begin(), dir.end());

		for(unsigned int i=0; i<files.size(); ++i)
		{
			Glib::ustring filename = Glib::build_filename(path, files[i]);

			if(Glib::file_test(filename, Glib::FILE_TEST_IS_DIR))
				load_path(filename, fhs_directory);
			else if(re->match(filename))
				load_extension_info(filename, fhs_directory);
		}
	}
	catch(const Glib::Error &ex)
	{
		se_debug_message(SE_DEBUG_APP, "error: %s", ex.what().c_str());
		std::cerr << ex.what() << std::endl;
	}
}

/*
 * Try to load an ExtensionInfo file.
 */
bool ExtensionManager::load_extension_info(const Glib::ustring &file, bool fhs_directory)
{
	se_debug_message(SE_DEBUG_APP, "try to read '%s'", file.c_str());

	try
	{
		Glib::KeyFile keyfile;
		if(keyfile.load_from_file(file) == false)
			throw SubtitleError(Glib::ustring::compose("Could not open the ExtensionInfo '%1'", file));

		// Check if it is a good file
		if(keyfile.has_group("SubtitleEditor Extension") == false)
			throw SubtitleError(Glib::ustring::compose("Bad extension file '%1'", file));

		// Get values
		Glib::ustring name, label, description, categorie, type, module, authors;
		bool hidden = false;

		// Get Name
		name = keyfile.get_string("SubtitleEditor Extension", "Name");
		if(name.empty())
			throw SubtitleError(Glib::ustring::compose("Could not find 'Name' in %1", file));

		// Get Label
		label = keyfile.get_locale_string("SubtitleEditor Extension", "Name");
		if(label.empty())
			throw SubtitleError(Glib::ustring::compose("Could not find 'Name' in %1", file));

		// Get Description
		description = keyfile.get_locale_string("SubtitleEditor Extension", "Description");
		if(description.empty())
			throw SubtitleError(Glib::ustring::compose("Could not find 'Description' in %1", file));

		// Get Categorie
		categorie = keyfile.get_string("SubtitleEditor Extension", "Categorie");
		if(categorie.empty())
			throw SubtitleError(Glib::ustring::compose("Could not find 'Categorie' in %1", file));

		// Get Type
		type = keyfile.get_string("SubtitleEditor Extension", "Type");
		if(type.empty())
			throw SubtitleError(Glib::ustring::compose("Could not find 'Type' in %1", file));

		// Get Module
		module = keyfile.get_string("SubtitleEditor Extension", "Module");
		if(module.empty())
			throw SubtitleError(Glib::ustring::compose("Could not find 'Module' in %1", file));

		// Get Hidden
		if(keyfile.has_key("SubtitleEditor Extension", "Hidden"))
			hidden = keyfile.get_boolean("SubtitleEditor Extension", "Hidden");

		// Get Authors
		authors = keyfile.get_locale_string("SubtitleEditor Extension", "Authors");

		// If a plugin with this name has already been loaded 
		// drop this one (user plugins override system plugins)
		if(get_extension_info(name) != NULL)
		{
			throw SubtitleError(Glib::ustring::compose(
						"Two or more plugins named '%1'. Only the first will be considered.", 
						name));
		}

		// Create the extension and init
		ExtensionInfo* info = new ExtensionInfo;

		info->file = file;
		info->name = name;
		info->label = label;
		info->description = description;
		info->categorie = categorie;
		info->type = type;
		info->module_name = module;
		info->authors = authors;
		info->hidden = hidden;
		info->fhs_directory = fhs_directory;

		// Append to the list
		m_extension_info_map[categorie].push_back(info);

		// Display Debug information
		se_debug_message(SE_DEBUG_APP, Glib::ustring::compose(
					"New ExtensionInfo: '%1' '%2' '%3' '%4'",
					name,	categorie, type,	module).c_str());

		return true;
	}
	catch(const Glib::Exception &ex)
	{
		se_debug_message(SE_DEBUG_APP, "error: %s", ex.what().c_str());
		std::cerr << "Error:" << ex.what() << std::endl;
	}
	catch(const std::exception &ex)
	{
		se_debug_message(SE_DEBUG_APP, "error: %s", ex.what());
		std::cerr << "Error:" << ex.what() << std::endl;
	}
	
	std::cerr << "Failed to read " << file << std::endl;

	return false;
}

/*
 * Return All ExtensionInfo.
 */
std::list<ExtensionInfo*> ExtensionManager::get_extension_info_list()
{
	se_debug(SE_DEBUG_APP);
	std::list<ExtensionInfo*> list;

	ExtensionInfoMap::iterator it_map;
	for(it_map = m_extension_info_map.begin(); it_map != m_extension_info_map.end(); ++it_map)
	{
		list.insert(list.end(), (*it_map).second.begin(), (*it_map).second.end());
	}

	return list;
}

/*
 * Return all ExtensionInfo in the categorie.
 */
std::list<ExtensionInfo*> ExtensionManager::get_info_list_from_categorie(const Glib::ustring &categorie)
{
	// FIXME
	std::list<ExtensionInfo*> list = m_extension_info_map[categorie];
	se_debug_message(SE_DEBUG_APP, "categorie='%s' size='%d'", categorie.c_str(), list.size());
		
	return list;
}

/*
 * Return an ExtensionInfo from this name or NULL if failed.
 */
ExtensionInfo* ExtensionManager::get_extension_info(const Glib::ustring &name)
{
	se_debug_message(SE_DEBUG_APP, "name='%s'", name.c_str());

	ExtensionInfoMap::iterator it_map;
	for(it_map = m_extension_info_map.begin(); it_map != m_extension_info_map.end(); ++it_map)
	{
		std::list<ExtensionInfo*>::iterator it_list;
		for(it_list = (*it_map).second.begin(); it_list != (*it_map).second.end(); ++it_list)
			if((*it_list)->name == name)
				return *it_list;
	}
	return NULL;
}

/*
 * Enable or disable extension.
 */
bool ExtensionManager::set_extension_active(const Glib::ustring &name, bool state)
{
	se_debug_message(SE_DEBUG_APP, "name='%s' active='%d'", name.c_str(), state);

	ExtensionInfo* info = get_extension_info(name);

	if(info == NULL)
		return false;


	bool res = (state) ? activate(info) : deactivate(info);
	if(!res)
	{
		se_debug_message(SE_DEBUG_APP, "Failed to change the extansion state");
		return false;
	}
	
	Config::getInstance().set_value_string("extension-manager", name, (state) ? "enable" : "disable");
	
	se_debug_message(SE_DEBUG_APP, "extension state is changed with success");
	return true;
}

/*
 * Try to activate the extension.
 * Load and create Extension.
 */
bool ExtensionManager::activate(ExtensionInfo *info)
{
	se_debug_message(SE_DEBUG_APP, "extension '%s'", info->get_name().c_str());

	// FIXME: add available value to info.
	try
	{
		// FIXME: 
		//if(info->type == "module")
		open_module(info);

		info->active = true;

		return true;
	}
	catch(const SubtitleError &ex)
	{
		se_debug_message(SE_DEBUG_APP, "activate the extension failed: %s", ex.what());
		std::cerr << ex.what() << std::endl;
	}
	catch(const Glib::Error &ex)
	{
		se_debug_message(SE_DEBUG_APP, "activate the extension failed: %s", ex.what().c_str());
	}
	catch(...)
	{
		se_debug_message(SE_DEBUG_APP, "activate the extension failed");
	}

	return false;
}

/*
 * Deactivate the extension.
 * Delete the extension and the module.
 */
bool ExtensionManager::deactivate(ExtensionInfo *info)
{
	se_debug_message(SE_DEBUG_APP, "extension '%s'", info->get_name().c_str());

	if(info->module == NULL || info->extension == NULL)
	{
		se_debug_message(SE_DEBUG_APP, "The Module or the Extension are NULL");
		return false;
	}

	try
	{
		se_debug_message(SE_DEBUG_APP, "delete extension...");
		
		if(info->extension)
			delete info->extension;
		info->extension = NULL;

		se_debug_message(SE_DEBUG_APP, "delete module...");
	
		if(info->module)
			delete info->module;
		info->module = NULL;
	}
	catch(...)
	{
		se_debug_message(SE_DEBUG_APP, "Error unknow exception!");
	}

	info->active = false;

	se_debug_message(SE_DEBUG_APP, "extension deactivate with success");
	return true;
}

/*
 * Open a module and create the extension.
 * If failed return a SubtitleError.
 */
void ExtensionManager::open_module(ExtensionInfo *info)
{
	se_debug(SE_DEBUG_APP);

	if(info->type != "module")
		throw SubtitleError("The type of the extension is not a 'module'");

	typedef Extension* (*ExtensionRegisterFunc) (void);
	
	Glib::ustring dirname = Glib::path_get_dirname(info->file);

	// It's only used for reading plugin without installing SE
	if(Glib::getenv("SE_DEV") == "1")
	{
		// ext/.libs/libext.so
		dirname = Glib::build_filename(dirname, ".libs");
	}
	else if(info->fhs_directory)
	{
		// If the extension is installed in the system, 
		// Filesystem Hierarchy Standard is used for the directory
		// The description and the module are not in the same directory
		utility::replace(dirname, PACKAGE_PLUGIN_DESCRIPTION_DIR, PACKAGE_PLUGIN_LIB_DIR);
	}

	// Build module name (path/libname.so)
	Glib::ustring file = Glib::Module::build_path(dirname, info->module_name);

	se_debug_message(SE_DEBUG_APP, "try to open module '%s'", file.c_str());

	// Create the module
	Glib::Module *module = new Glib::Module(file);
	if(!*module)
	{
		throw SubtitleError(
				Glib::ustring::compose("Failed to create the Glib::Module: %1", 
					Glib::Module::get_last_error()));
	}

	// Get the register function
	void *func = NULL;
	if(module->get_symbol("extension_register", func) == false)
	{
		throw SubtitleError(
				Glib::ustring::compose("Failed to get the extension_register function: %1", 
					Glib::Module::get_last_error()));
	}

	//Fix: bug #12651 : 0.30.0 build error
	//ExtensionRegisterFunc extension_register = (ExtensionRegisterFunc)func;
	ExtensionRegisterFunc extension_register = reinterpret_cast<ExtensionRegisterFunc>(func);
	
	if(extension_register == NULL)
		throw SubtitleError("reinterpret from the function to the ExtensionRegisterFunc failed");

	// create the extension
	Extension *ext = extension_register();

	if(ext == NULL)
		throw SubtitleError("Could not create Extension, extension_register return NULL");

	info->module = module;
	info->extension = ext;

	se_debug_message(SE_DEBUG_APP, "Opening and the creating the extension from the module is a success");
}

