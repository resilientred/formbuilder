/**************************************************************************************************
 *                                                                                                *
 *    Project:  NextGIS Formbuilder                                                               *
 *    Authors:  Mikhail Gusev, gusevmihs@gmail.com                                                *
 *              Copyright (C) 2014-2018 NextGIS                                                   *
 *                                                                                                *
 *    This program is free software: you can redistribute it and/or modify it under the terms     *
 *    of the GNU General Public License as published by the Free Software Foundation, either      *
 *    version 2 of the License, or (at your option) any later version.                            *
 *                                                                                                *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;   *
 *    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   *
 *    See the GNU General Public License for more details.                                        *
 *                                                                                                *
 *    You should have received a copy of the GNU General Public License along with this program.  *
 *    If not, see http://www.gnu.org/licenses/.                                                   *
 *                                                                                                *
 **************************************************************************************************/

#include "fb_window.h"

#include "core/all_registry.h"
#include "gui/all_registry.h"
#include "mockup/all_registry.h"
#include "util/settings.h"

#include "ogrsf_frmts.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QTextCodec>


void installTranslators (const QApplication &app, const QStringList &tr_names)
{
    QStringList tr_paths;
    #ifdef Q_OS_MACOS

    #else
//    const QString &app_tr_path = QCoreApplication::applicationDirPath()
//            + QLatin1String("/../share/translations");

    // TEMP:
    const QString &app_tr_path = QCoreApplication::applicationDirPath();
    tr_paths.append(app_tr_path);
    #endif

    tr_paths.append(QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    for (auto &tr_path: tr_paths)
    {
        QDir dir(tr_path);
        QStringList tr_file_names = dir.entryList(tr_names);
        for (auto &tr_file_name: tr_file_names)
        {
            QString file = dir.absoluteFilePath(tr_file_name);
            QTranslator *translator = new QTranslator();
            if (translator->load(file))
            {
                qDebug() << "Loaded translation file: " << file;
                app.installTranslator(translator);
            }
            else
            {
                qDebug() << "Unable to load translation file: " << file;
                delete translator;
            }
        }
    }
}


int main (int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("NextGIS");
    QCoreApplication::setApplicationName("Formbuilder");
    QCoreApplication::setOrganizationDomain("com.nextgis");
//    QCoreApplication::setApplicationVersion();

    // Parse command line options.
//    QCommandLineParser parser;
//    parser.setApplicationDescription(QCoreApplication::applicationName());
//    parser.addHelpOption();
//    parser.addVersionOption();
//    parser.addPositionalArgument("file", "The file to open.");
//    parser.process(app);

    // Read settings.
    QString language_code = g_getSettings()->value(FB_STS_LANGUAGE, "").toString();
    Language language = g_findLanguage(language_code);
    if (language == Language::Undefined)
    {
        qDebug() << QString("Unknown language code: %1").arg(language_code);
        language = Language::en_GB;
    }
    LanguageData language_data = LANGUAGES.value(language);

    // Install translators.
    QStringList translator_names;
    translator_names << QStringLiteral("ngstd_*%1*").arg(language_data.sys_code);
    translator_names << QStringLiteral("qt_%1*").arg(language_data.sys_code);
    translator_names << QStringLiteral("qtbase_%1*").arg(language_data.sys_code);
    translator_names << QStringLiteral("fb_%1*").arg(language_data.code);
    installTranslators(app, translator_names);

    // Qt specific: for using *.qrc files when linking statically with fb libraries.
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(styles);
    Q_INIT_RESOURCE(resources);

    // The same for lib_ngstd:
//    Q_INIT_RESOURCE(framework);

    // Initialize GDAL.
    OGRRegisterAll();
    CPLSetConfigOption("CPL_VSIL_ZIP_ALLOWED_EXTENSIONS", "ngfp");

    // Register Formbuilder static data.
    Fb::Core::g_registerAll();
    Fb::Gui::g_registerAttrWatchers();
    Fb::Mockup::g_registerElemViews();

    // Show GUI.
    FbWindow window(language);
    window.show();

    int ret = app.exec();

    return ret;
}


