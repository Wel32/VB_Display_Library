

#ifdef __cplusplus
extern "C"
{
#endif



extern void sl_app_hwinit();
extern void sl_app_config_init();



void app_main() 
{
    sl_app_hwinit();
    sl_app_config_init();
}



#ifdef __cplusplus
}
#endif
