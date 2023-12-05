#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq ($(OS),Windows_NT)
S2SEXE=$(BSP_DIR)/projects/common/scripts/s2s.bat
LINK=&&
TOUCH=cmd /c type nul >
else
S2SEXE=$(BSP_DIR)/projects/common/scripts/s2s.sh
LINK=;
TOUCH=touch
endif

.PRECIOUS: .java.fp .java.mock .java.configuration .%.java.app

.java.fp:
	cd $(FP_DIR) $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm publish \
		-Declipse.home="$(ECLIPSE_HOME)" \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		$(MODULE_REPOSITORY_SETTINGS) \
	 $(JAVA_VERBOSE)
	$(TOUCH) $@

.java.mock:
	cd $(MOCK_DIR) $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm publish \
		-Declipse.home="$(ECLIPSE_HOME)" \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		$(MODULE_REPOSITORY_SETTINGS) \
		$(JAVA_VERBOSE)
	$(TOUCH) $@

.java.configuration:
	cd $(CONF_DIR) $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm publish  \
		$(PUBLISH_ARTIFACTS) \
		-Declipse.home="$(ECLIPSE_HOME)" \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		$(MODULE_REPOSITORY_SETTINGS) \
		$(JAVA_VERBOSE)
	$(TOUCH) $@

.java.test.s2s:
	$(S2SEXE) $(S2S_TTY) $(S2S_PORT) &

javasec.prop :
	$(MICROEJ_SEC)

.java.test.core: javasec.prop
	cd $(VAL_DIR)/core/java-testsuite-runner-core  $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm publish  \
		-Declipse.home="$(ECLIPSE_HOME)" \
		$(MODULE_REPOSITORY_SETTINGS) \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		-Dplatform-loader.target.platform.dir=$(PLAT_DIR)/source \
		-Ddeploy.bsp.root.dir=$(BSP_DIR) \
		-Dlaunch.properties.jvm="-Djava.security.properties=$(BASE_DIR)/javasec.prop" \
		-Dplatform-launcher.platform.dir=$(PLAT_DIR)/source $(JAVA_VERBOSE)

.java.test.ui: javasec.prop
	cd $(VAL_DIR)/ui/ui3/java-testsuite-runner-ui3  $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm publish  \
		-Declipse.home="$(ECLIPSE_HOME)" \
		$(MODULE_REPOSITORY_SETTINGS) \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		-Dplatform-loader.target.platform.dir=$(PLAT_DIR)/source \
		-Ddeploy.bsp.root.dir=$(BSP_DIR) \
		-Dlaunch.properties.jvm="-Djava.security.properties=$(BASE_DIR)/javasec.prop" \
		-Dplatform-launcher.platform.dir=$(PLAT_DIR)/source $(JAVA_VERBOSE)

.%.java.app: javasec.prop
	cd $(APP_DIR) $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm publish  \
		-Declipse.home="$(ECLIPSE_HOME)" \
		$(MODULE_REPOSITORY_SETTINGS) \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		-Dplatform-loader.target.platform.dir=$(PLAT_DIR)/source \
		-Dplatform-launcher.platform.dir=$(PLAT_DIR)/source \
		-Dapplication.main.class=$(if $(MAIN),$(MAIN),$(MAIN$@)) \
		-Dlaunch.properties.jvm="-Djava.security.properties=$(BASE_DIR)/javasec.prop" \
		-Ddeploy.bsp.root.dir=$(BSP_DIR) $(JAVA_VERBOSE)
	$(call del_file,.*.java.app)
	$(TOUCH) $@

nxpvee-validation.prj: clean .java.fp .java.mock .java.configuration .java.test.s2s .java.test.core .java.test.ui
	echo "done"

%.prj: .java.fp .java.mock .java.configuration .%.java.app
	make -C $(BSP_DIR)/projects/$(basename $@)/sdk_makefile

$(addsuffix -flash,$(PROJS)):
	make $(@:%-flash=%).prj
	make -C $(BSP_DIR)/projects/$(@:%-flash=%)/sdk_makefile flash

$(addsuffix -flash_cmsisdap,$(PROJS)):
	make $(@:%-flash_cmsisdap=%).prj
	make -C $(BSP_DIR)/projects/$(@:%-flash_cmsisdap=%)/sdk_makefile flash_cmsisdap

$(addsuffix -gdb,$(PROJS)):
	make $(@:%-gdb=%).prj
	make -C $(BSP_DIR)/projects/$(@:%-gdb=%)/sdk_makefile gdb

$(addsuffix -gdb_cmsisdap,$(PROJS)):
	make $(@:%-gdb_cmsisdap=%).prj
	make -C $(BSP_DIR)/projects/$(@:%-gdb_cmsisdap=%)/sdk_makefile gdb_cmsisdap

$(addsuffix -java_run,$(PROJS)): .java.fp .java.mock .java.configuration
	cd $(APP_DIR)  $(LINK) $(MICROEJ_BUILDKIT_PATH_VAR)/bin/mmm run  \
		-Declipse.home="$(ECLIPSE_HOME)" \
		$(MODULE_REPOSITORY_SETTINGS) \
		-Dcom.microej.platformbuilder.architecture.usage=$(USAGE) \
		-Dplatform-loader.target.platform.dir=$(PLAT_DIR)/ \
		-Dapplication.main.class=$(if $(MAIN),$(MAIN),$(MAIN.$(@:%-java_run=%).java.app)) \
		-Dplatform-launcher.platform.dir=$(PLAT_DIR)/source $(JAVA_VERBOSE)

$(addsuffix -java_rebuild,$(PROJS)): .java.fp .java.mock .java.configuration
	$(call del_file,.*.java.app)
	make .$(@:%-java_rebuild=%).java.app

$(addsuffix -clean,$(PROJS)):
	make -C $(BSP_DIR)/projects/$(@:%-clean=%)/sdk_makefile clean
	make -C $(BSP_DIR)/projects/$(@:%-clean=%)/sdk_makefile distclean
	$(call del_file,.*.java.app)
	$(call del_file,.java.*)
	$(call del_file,javasec.prop*)
	$(call del_dir,$(PLAT_DIR)/build/)
	$(call del_dir,$(PLAT_DIR)/source/)
	$(call del_file,$(CONF_DIR)/dropins/imagegenerator-vectorimage.jar)
	$(call del_file,$(CONF_DIR)/dropins/license-checker.jar)
	$(call del_dir,$(CONF_DIR)/target~/)
	$(call del_dir,$(CONF_DIR)/dropins/include)
	$(call del_dir,$(CONF_DIR)/dropins/javaLibs)
	$(call del_dir,$(CONF_DIR)/dropins/mocks)
	$(call del_dir,$(CONF_DIR)/dropins/scripts)
	$(call del_file,$(CONF_DIR)/dropins/workbenchExtension_launchScriptFramework.jar)
	$(call del_dir,$(FP_DIR)/bin/)
	$(call del_dir,$(FP_DIR)/lib/)
	$(call del_dir,$(FP_DIR)/target~/)
	$(call del_dir,$(APP_DIR)/bin/)
	$(call del_dir,$(APP_DIR)/filesystem/)
	$(call del_dir,$(APP_DIR)/src-adpgenerated/)
	$(call del_dir,$(APP_DIR)/target~/)
	$(call del_dir,$(BSP_DIR)/projects/microej/platform/)
	$(call del_dir,$(MOCK_DIR)/bin/)
	$(call del_dir,$(MOCK_DIR)/target~/)
	$(call del_dir,$(VAL_DIR)/core/java-testsuite-runner-core/target~/)
	$(call del_dir,$(VAL_DIR)/fs/)

clean: $(addsuffix -clean,$(PROJS))
	echo "clean"
