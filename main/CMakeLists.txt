idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver Task1000ms Task100ms Task10ms WindowManager ButtonManager ScreenManager)