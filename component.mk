# Make sure to put the config manager in the ../components folder.
# Once this is done you should have the following directory structure
#   ../components/
#         |
#		  - ConfigManager/
#                      |
#                       - component.mk
#                       - src/
#                            |
#                            --- your cpp files
#                       - include/
#                            |
#                            --- your header files
COMPONENT_SEARCH_DIRS = ../components

# And don't forget specify that the application depends on it.
COMPONENT_DEPENDS := ConfigManager

HWCONFIG := spiffs