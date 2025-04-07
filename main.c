
#include <genesis.h>
#include <resources.h>

s16 current_camera_x;
s16 current_camera_y;
s16 new_camera_x;
s16 new_camera_y;

u16 palette_full[64];


#define HOW_FAR_TO_LEFT_BEFORE_CAMERA_MOVES 150
#define HOW_FAR_TO_RIGHT_BEFORE_CAMERA_MOVES 151
#define HOW_FAR_TO_TOP_BEFORE_CAMERA_MOVES 75
#define HOW_FAR_TO_BOTTOM_BEFORE_CAMERA_MOVES 76

#define HORIZONTAL_RESOLUTION 320
#define VERTICAL_RESOLUTION 224

#define MAP_WIDTH 2240
#define MAP_HEIGHT 656

#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 64

Map* level_map;
u16 ind = TILE_USER_INDEX;

Sprite* adam_sprite;
fix32 adam_x = FIX32(608);
fix32 adam_y = FIX32(300);
fix32 adam_vel = FIX32(3.5);


static void handleInput();
static void camera();
static void setEnemyPosition(Sprite* enemy_sprite, fix32 enemy_x, fix32 enemy_y);


//HERE////////////////////////////////////////////////////////////////////////////////////////////////////

s16 screen_water_level = 0;
s16 map_water_level = 300;

HINTERRUPT_CALLBACK HIntHandler()
{
    VDP_setHilightShadow(TRUE);
}

void VBlankHandler()
{
    VDP_setHilightShadow(FALSE);
    VDP_setHIntCounter(screen_water_level);
}

//END/////////////////////////////////////////////////////////////////////////////////////////////////////



int main()
{

    memcpy(&palette_full[0], background_image.palette->data, 16 * 8);

    SPR_init();

    VDP_loadTileSet(&level2_foreground_tileset, ind, DMA);
    level_map = MAP_create(&level2_foreground_map, BG_A, ind); 
    ind += level2_foreground_tileset.numTile;
    VDP_drawImageEx(BG_B, &background_image, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);

    MAP_scrollTo(level_map, 0, 0);
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

    adam_sprite = SPR_addSprite(&adam_img, fix32ToInt(adam_x), fix32ToInt(adam_y), TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    //HERE////////////////////////////////////////////////////////////////////////////////////////////////////

    SYS_disableInts();
    {
        SYS_setVBlankCallback(VBlankHandler);
        SYS_setHIntCallback(HIntHandler);        
        VDP_setHInterrupt(TRUE);     
    }
    SYS_enableInts();

    //END/////////////////////////////////////////////////////////////////////////////////////////////////////

    PAL_fadeIn(0, 63, palette_full, 10, FALSE);
    
    while(1)
    {
        handleInput();
        camera();

        KLog_U1("screen water level: ", screen_water_level);

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}


static void handleInput() {
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_LEFT) {
        adam_x -= adam_vel;

    }
    else if (value & BUTTON_RIGHT) {
        adam_x += adam_vel;

    }
    if (value & BUTTON_UP) {
        adam_y -= adam_vel;

    }
    else if (value & BUTTON_DOWN) {
        adam_y += adam_vel;

    }
}

static void camera() {

    if (adam_x < FIX32(0)) adam_x = FIX32(0);
    else if (adam_x > FIX32(MAP_WIDTH - PLAYER_WIDTH)) adam_x = FIX32(MAP_WIDTH - PLAYER_WIDTH);
    if (adam_y < FIX32(0)) adam_y = FIX32(0);
    else if (adam_y > FIX32(MAP_HEIGHT - PLAYER_HEIGHT)) adam_y = FIX32(MAP_HEIGHT - PLAYER_HEIGHT); 


    s16 player_x_map_integer = fix32ToInt(adam_x);
    s16 player_y_map_integer = fix32ToInt(adam_y);

    s16 player_x_position_on_screen = player_x_map_integer - current_camera_x;
    s16 player_y_position_on_screen = player_y_map_integer - current_camera_y;


    if (player_x_position_on_screen > HOW_FAR_TO_RIGHT_BEFORE_CAMERA_MOVES) {
        new_camera_x = player_x_map_integer - HOW_FAR_TO_RIGHT_BEFORE_CAMERA_MOVES;
    }
    else if (player_x_position_on_screen < HOW_FAR_TO_LEFT_BEFORE_CAMERA_MOVES) {
        new_camera_x = player_x_map_integer - HOW_FAR_TO_LEFT_BEFORE_CAMERA_MOVES;
    }
    else new_camera_x = current_camera_x;

    if (player_y_position_on_screen > HOW_FAR_TO_BOTTOM_BEFORE_CAMERA_MOVES) {
        new_camera_y = player_y_map_integer - HOW_FAR_TO_BOTTOM_BEFORE_CAMERA_MOVES;
    }
    else if (player_y_position_on_screen < HOW_FAR_TO_TOP_BEFORE_CAMERA_MOVES) {
        new_camera_y = player_y_map_integer - HOW_FAR_TO_TOP_BEFORE_CAMERA_MOVES;
    }
    else new_camera_y = current_camera_y;

    if (new_camera_x < 0) new_camera_x = 0;
    else if (new_camera_x > (MAP_WIDTH - HORIZONTAL_RESOLUTION)) new_camera_x = MAP_WIDTH - HORIZONTAL_RESOLUTION;
    if (new_camera_y < 0) {
        new_camera_y = 0;
    }
    else if (new_camera_y > (MAP_HEIGHT - VERTICAL_RESOLUTION)) {
        new_camera_y = MAP_HEIGHT - VERTICAL_RESOLUTION;
    }

    s16 b_hscroll;
    s16 b_vscroll;

    if ( (current_camera_x != new_camera_x) || (current_camera_y != new_camera_y)) {
        current_camera_x = new_camera_x;
        current_camera_y = new_camera_y;

        //HERE////////////////////////////////////////////////////////////////////////////////////////////////////

        screen_water_level = (map_water_level - new_camera_y);
        if (new_camera_y > map_water_level) screen_water_level = 0;
        if (screen_water_level > 224) screen_water_level = 224;

        //END/////////////////////////////////////////////////////////////////////////////////////////////////////

        MAP_scrollTo(level_map, new_camera_x, new_camera_y);
        b_hscroll = 0 - (new_camera_x >> 2);
        b_vscroll = new_camera_y >> 4;
        if (b_vscroll < 0) b_vscroll = 0;
        VDP_setHorizontalScroll(BG_B, b_hscroll);
        VDP_setVerticalScroll(BG_B, b_vscroll);
    }
    

    SPR_setPosition(adam_sprite, fix32ToInt(adam_x) - new_camera_x, fix32ToInt(adam_y) - new_camera_y);
    
}

static void setEnemyPosition(Sprite* enemy_sprite, fix32 enemy_x, fix32 enemy_y) {

    s16 enemy_x_as_s16 = fix32ToInt(enemy_x) - new_camera_x;
    s16 enemy_y_as_s16 = fix32ToInt(enemy_y) - new_camera_y;

    if (enemy_x_as_s16 < -56 || enemy_x_as_s16 > HORIZONTAL_RESOLUTION 
     || enemy_y_as_s16 < -80 || enemy_y_as_s16 > VERTICAL_RESOLUTION) {

        SPR_setVisibility(enemy_sprite, HIDDEN);

    }
    else {
        SPR_setVisibility(enemy_sprite, VISIBLE);
        SPR_setPosition(enemy_sprite, enemy_x_as_s16, enemy_y_as_s16);
    }

}
