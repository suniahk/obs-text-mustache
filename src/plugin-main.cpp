/*
Plugin Name
Copyright (C) 2024 Mark Jones <mark.jones1112+nospam@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-support.h>
#include <QMainWindow>
#include <QWidget>
#include <QDockWidget>
#include <QString>
#include <obs-frontend-api.h>
#include <obs.hpp>
#include <util/util.hpp>
#include <util/platform.h>
#include "obs-text-mustache-definitions.hpp"
#include "plugin-lifetime.hpp"
#include "types.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load()
{
	InitOBSText();
	auto *window = (QMainWindow *)obs_frontend_get_main_window();

	obs_frontend_push_ui_translation(obs_module_get_string);

	auto *obsTextMustache = new OBSTextMustacheDefinitions(window);

	//const QString title = QString::fromUtf8(obs_module_text("Text Template Values"));
	const QString title = QString::fromUtf8(obs_module_text("OBSTextMustacheDefinitions"));
	const auto name = "OBSTextMustacheDefinitions";

	obs_frontend_add_dock_by_id(name, title.toUtf8().constData(),
				    obsTextMustache);

	obs_frontend_pop_ui_translation();

	blog(LOG_INFO, "obs-text-mustache-definitions loaded successfully.");
	return true;
}

void obs_module_unload()
{
	FreeOBSText();
	obs_log(LOG_INFO, "plugin unloaded");
}

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Windows GDI+ text source with templating";
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return obs_module_text("OBSTextMustache");
}