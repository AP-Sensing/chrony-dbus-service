
PROJ_NAME_SNAKE_UPPERCASE="CHRONY_DBUS_SERVICE"
PROJ_NAME_SNAKE_LOWERCASE="chrony_dbus_service"
PROJ_NAME_CAMEL_CASE="ChronyDBusService"
PROJ_NAME_LOWERCASE="chronydbusservice"

# * Do an overall text replace of `aps_template_option` with the name of your project in snake case followed by `_option` and prefixed by `aps_`. In case your project is called `FancyTool`, replace it with `aps_fancytool_option`.
grep -lr --null "aps_template_option" . | xargs --null sed -i "s/aps_template_option/aps_${PROJ_NAME_LOWERCASE}_option/g"
# 4. Replace all occurrences of `APS_TEMPLATE_` (case sensitive!) in you whole project with `APS_`, followed by your project name in upper case and then an `_`. In case your project is called `FancyTool`, replace it with `APS_FANCY_TOOL_`.
grep -lr --null "APS_TEMPLATE_" . | xargs --null sed -i "s/APS_TEMPLATE_/APS_${PROJ_NAME_SNAKE_UPPERCASE}_/g"
# 5. Replace all occurrences of `aps_interrogator_` (case sensitive!) in you whole project with `aps_`, followed by your project name in lower case and then an `_`. In case your project is called `FancyTool`, replace it with `aps_fancytool_`.
grep -lr --null "aps_interrogator_" . | xargs --null sed -i "s/aps_interrogator_/aps_${PROJ_NAME_LOWERCASE}_/g"
# 6. Replace all occurrences of `aps::interrogator` (case sensitive!) in you whole project with `aps::`, followed by your project name in lower case. In case your project is called `FancyTool`, replace it with `aps::fancytool`.
grep -lr --null "aps::interrogator" . | xargs --null sed -i "s/aps::interrogator/aps::${PROJ_NAME_LOWERCASE}/g"
# 7. Replace all occurrences of `aps/interrogator` (case sensitive!) in you whole project with `aps/`, followed by your project name in lower case. In case your project is called `FancyTool`, replace it with `aps/fancytool`.
grep -lr --null "aps/interrogator" . | xargs --null sed -i "s|aps/interrogator|aps/${PROJ_NAME_LOWERCASE}|g"
# 11. Replace all occurrences of `add_interrogator_test_executable` in you whole project with `add_`, followed by your project name in snake case and then `_test_executable`. In case your project is called `FancyTool`, replace it with `add_fancy_tool_test_executable`.
grep -lr --null "add_interrogator_test_executable" . | xargs -0 sed -i "s/add_interrogator_test_executable/add_${PROJ_NAME_SNAKE_LOWERCASE}_test_executable/g"
# 12. Replace all occurrences of `interrogator` (case sensitive!) in you whole project with your project name in lower case. In case your project is called `FancyTool`, replace it with `fancytool`.
grep -lr --null "interrogator" . | xargs --null sed -i "s/interrogator/${PROJ_NAME_LOWERCASE}/g"
# 13. Replace all occurrences of `Interrogator` (case sensitive!) in you whole project with your project name in upper camel case. In case your project is called `FancyTool`, replace it with `FancyTool`.
grep -lr --null "Interrogator" . | xargs --null sed -i "s/Interrogator/${PROJ_NAME_CAMEL_CASE}/g"
# 14. Replace all occurrences of `INTERROGATOR` (case sensitive!) in you whole project with your project name in upper snake case. In case your project is called `FancyTool`, replace it with `FANCY_TOOL`.
grep -lr --null "INTERROGATOR" . | xargs --null sed -i "s/INTERROGATOR/${PROJ_NAME_SNAKE_UPPERCASE}/g"


# 9. Rename the file `src/aps/<nameofyourproject>/res/interrogator.resx` to your project name in lower case. In case your project is called `FancyTool`, rename them to `src/aps/<nameofyourproject>/res/fancytool.resx`.
mv src/aps/interrogator/res/interrogator.resx src/aps/interrogator/res/${PROJ_NAME_LOWERCASE}.resx
# 10. Rename the file `src/aps/<nameofyourproject>/protocol/InterrogatorApi.yml` to your project name in PascalCase followed by `Api.yml`. In case your project is called `FancyTool`, rename them to `src/aps/<nameofyourproject>/protocol/FancyToolApi.yml`.
mv src/aps/interrogator/protocol/InterrogatorApi.yml src/aps/interrogator/protocol/${PROJ_NAME_CAMEL_CASE}.yml
# 8. Rename the directories `src/aps/interrogator` and `src/include/aps/interrogator` accordingly to your namespaces. In case your project is called `FancyTool`, rename them to `src/aps/fancytool` and `src/include/aps/fancytool`.
mv src/aps/interrogator src/aps/${PROJ_NAME_LOWERCASE}
mv src/include/aps/interrogator src/include/aps/${PROJ_NAME_LOWERCASE}

# Undo modifications to this script
git restore $0
