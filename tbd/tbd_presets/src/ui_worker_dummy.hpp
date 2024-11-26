namespace tbd::favorites {

FavoritesModel model;
int32_t active_fav = -1;
int32_t selected_fav = -1;

struct UIWorkerDummy {
    void begin() {}
    void clear() {}
    void enable() {}
    void disable() {}

} ui_worker;

}
