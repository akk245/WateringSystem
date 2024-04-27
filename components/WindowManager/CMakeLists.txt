idf_component_register(SRCS "WindowManager.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES PriorityQueue MenuManager nvs_flash
                    REQUIRES driver)