Prepare : BuildPeiInitTable BuildSioElink SioLink_inf SioLinkLibs_h
 
#---------------------------------------------------------------------------
#       Generate SIO Pei elink table
#       1. Generate SIO PEI "IO Decode table"
#       2. Generate SIO PEI "IO initialization table"
#       3. Generate SIO PEI "Debug IO Decode table"
#       4. Generate SIO PEI "Debug IO initialization table"
#---------------------------------------------------------------------------
BuildPeiInitTable : $(SIO_PEI_TABLE_DEFINITIONS) $(BUILD_DIR)/token.mak
	$(ECHO) \
"$(if $(SIO_PEI_TABLE_DEFINITIONS), $(foreach S_DEF, $(SIO_PEI_TABLE_DEFINITIONS),#include<$(S_DEF)>$(EOL)))"\
>$(BUILD_DIR)/PrivateSioPeiInitTable.h

#---------------------------------------------------------------------------
#       Generate SIO elink table
#		1. SIO init string table for SioSetup.c
#---------------------------------------------------------------------------
BuildSioElink : $(BUILD_DIR)/token.mak
	$(ECHO) \
"\
#ifndef _SIO_ELINK_H_$(EOL)\
#define _SIO_ELINK_H_$(EOL)\
$(EOL)\
#define SIO_Init_Str_LIST $(SioStrInitList)$(EOL)\
#endif$(EOL)\
"> $(BUILD_DIR)\SIOElink.h



Prepare :  $(BUILD_DIR)/SioLinkLib.inf SioLinkLibs_h $(BUILD_DIR)/SioLinkLibs.h

#---------------------------------------------------------------------------
#       Generate SIO LibraryInstance INF file
#		1. Generate INF basic infromation
#       2. Define C file elink
#---------------------------------------------------------------------------
SioLink_inf : 
	$(ECHO)  \
"[Defines]$(EOL)\
  INF_VERSION                    = 0x00010005$(EOL)\
  BASE_NAME                      = SioLinkLib$(EOL)\
  FILE_GUID                      = E998C6D8-572B-4e18-96CC-031EA3DD5581$(EOL)\
  MODULE_TYPE                    = BASE$(EOL)\
  VERSION_STRING                 = 1.0$(EOL)\
  LIBRARY_CLASS                  = SioLinkLib$(EOL)\
$(EOL)\
[Sources]$(EOL)\
  $(patsubst %,%,$(subst $(SPACE),$(EOL)$(SPACE),$(SIOLINK_LIB_SRC)))$(EOL)\
  $(EOL)\
[Packages] $(EOL)\
  MdePkg/MdePkg.dec$(EOL)\
  IntelFrameworkPkg/IntelFrameworkPkg.dec$(EOL)\
  AmiCompatibilityPkg/AmiCompatibilityPkg.dec$(EOL)\
"\
> $(BUILD_DIR)/SioLinkLib.inf

#---------------------------------------------------------------------------
#       Generate SIO LibraryInstance H file
#       1. Define H file elink
#---------------------------------------------------------------------------
SioLinkLibs_h : 
	$(ECHO)  \
	$(patsubst %,#include \"%\"$(EOL),$(SIOLINK_LIB_H))\
> $(BUILD_DIR)/SioLinkLibs.h
