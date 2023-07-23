idf_component_register(SRCS "MenuManager.c" "getTimeSM.c" "MenuUtilities.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES ScreenManager ButtonManager WindowManager Utilities PriorityQueue
                    REQUIRES driver)