
#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <rafgl.h>
#include <game_constants.h>

static rafgl_raster_t doge;
static rafgl_raster_t upscaled_doge;
static rafgl_raster_t raster, raster2;
static rafgl_raster_t checker;
static rafgl_texture_t texture;
static rafgl_spritesheet_t hero;
static rafgl_spritesheet_t enemy;
static rafgl_spritesheet_t projectile;
static rafgl_spritesheet_t enemy_projectile;
static rafgl_spritesheet_t explosion;

static int w, h;

#define NUMBER_OF_TILES 6
rafgl_raster_t tiles[NUMBER_OF_TILES];

#define WORLD_SIZE 128*10
int tile_world[WORLD_SIZE][WORLD_SIZE];

#define TILE_SIZE 64

static int raster_width = RASTER_WIDTH, raster_height = RASTER_HEIGHT;



#define MAX_ENEMY 5

typedef struct enemyS
{

    float enemy_pos_y;
    float enemy_pos_x;
    int alive;
    float enemy_pos_y_dead;
    float enemy_pos_x_dead;
    int shot;
    int direction;
    int time;

} enemyS;

enemyS enemies[MAX_ENEMY];

#define MAX_PARTICLES 500
typedef struct _particle_t
{
    float x, y, dx, dy;
    int life;
    int direction;

} particle_t;

typedef struct _enemy_particle_t
{
    float x, y, dx, dy;
    int life;
    int direction;

} enemy_particle_t;
typedef struct _sad_particle_t
{
    float x, y, dx, dy;
    int life;
    float speed;


} sad_particle_t;

// sad_particles su partikli kada se ubrzamo
particle_t particles[MAX_PARTICLES];
enemy_particle_t enemy_particles[MAX_PARTICLES];
sad_particle_t sad_particles[MAX_PARTICLES];



static char save_file[256];
int save_file_no = 0;

int camx = WORLD_SIZE/2, camy = WORLD_SIZE*TILE_SIZE;

int selected_x, selected_y;


void draw_particles(rafgl_raster_t *raster)
{

    int i;
    particle_t p;
    for(i = 0; i < MAX_PARTICLES; i++)
    {

        p = particles[i];
        if(p.life <= 0) continue;


    }

}
void draw_sad_particles(rafgl_raster_t *raster)
{
    int i;
    sad_particle_t p;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
          p = sad_particles[i];

         //  p.x = 0;
      //  p.y = rand() % raster_height;
        //p.speed = (float)(rand() % 10 + 5) / 10.0;

         rafgl_raster_draw_line(raster, p.x - p.dx, p.y - p.dy, p.x, p.y, rafgl_RGB(255, 255,  0));
    }
}

void draw_enemy_particles(rafgl_raster_t *raster)
{

    int i;
    enemy_particle_t p;
    for(i = 0; i < MAX_PARTICLES; i++)
    {

        p = enemy_particles[i];
        if(p.life <= 0) continue;

    }

}

static float elasticity = 0.7;

void update_particles(float delta_time)
{
    int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(particles[i].life <= 0){
                    particles[i].x=-40;
                    particles[i].y=-40;
        }

        particles[i].life--;

        particles[i].x += particles[i].dx;
        particles[i].y += particles[i].dy;
        rafgl_raster_draw_spritesheet(&raster, &projectile, 1, 1, particles[i].x, particles[i].y);


    }

}
void update_enemy_particles(float delta_time)
{
    int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(enemy_particles[i].life <= 0){
                    enemy_particles[i].x=-40;
                    enemy_particles[i].y=+40;
        }

        enemy_particles[i].life--;

        enemy_particles[i].x += enemy_particles[i].dx;
        enemy_particles[i].y += enemy_particles[i].dy;
        rafgl_raster_draw_spritesheet(&raster, &enemy_projectile, 2, 2, enemy_particles[i].x, enemy_particles[i].y);


    }

}



void update_sad_particles(float delta_time)
{
     int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(sad_particles[i].life <= 0) continue;

        sad_particles[i].life--;

        sad_particles[i].x += sad_particles[i].dx;
        sad_particles[i].y += sad_particles[i].dy;
        sad_particles[i].dx *= 0.995f;
        sad_particles[i].dy *= 0.995f;
        sad_particles[i].dy += 0.05;

        //ukoliko smo udarili u levi zid
        //onda pomeramo da bude x na 0
        //i samo promenimo smer za dx
        //to smo mogli i bez abs samo sa minusom
        if(sad_particles[i].x < 0)
        {
            sad_particles[i].x = 0;
            //elasticity nam je gubitak u svakom slucaju, ali imamo i neki random gubitak
            //da se ne bi svi partikli odbijali istom jacinom od zida
            sad_particles[i].dx = (rafgl_abs_m(sad_particles[i].dx)) * randf() * elasticity;
        }


    }
}

void init_tilemap(void)
{

    int x, y;
    for(y = 0; y < WORLD_SIZE; y++)
    {
        for(x = 0; x < WORLD_SIZE; x++)
        {
            if(randf() <0.02f)
            {

                tile_world[y][x] =  rand() % 3;
            }
            else
            {
                tile_world[y][x] = 5;
            }
             if(randf() <0.002f)
            {

                tile_world[y][x] =  4;
            }
        }
    }
}



void render_tilemap(rafgl_raster_t *raster)
{
    int x, y;
    int x0 = camx / TILE_SIZE;

    int x1 = x0 + (raster_width / TILE_SIZE) + 1;
    int y0 = camy / TILE_SIZE;
    int y1 = y0 + (raster_height / TILE_SIZE) + 2;

    //klempujemo
    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;

    //klempujemo
    if(x0 >= WORLD_SIZE) x0 = WORLD_SIZE - 1;
    if(y0 >= WORLD_SIZE) y0 = WORLD_SIZE - 1;
    if(x1 >= WORLD_SIZE) x1 = WORLD_SIZE - 1;
    if(y1 >= WORLD_SIZE) y1 = WORLD_SIZE - 1;

    rafgl_raster_t *draw_tile;


    for(y = y0; y <= y1; y++)
    {
        for(x = x0; x <= x1; x++)
        {
            draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);
            rafgl_raster_draw_raster(raster, draw_tile, x * TILE_SIZE - camx, y * TILE_SIZE - camy - draw_tile->height + TILE_SIZE);
        }
    }




}


void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    rafgl_raster_load_from_image(&doge, "res/images/space1.png");
    rafgl_raster_load_from_image(&checker, "res/images/checker32.png");

    raster_width = width;
    raster_height = height;

    rafgl_raster_init(&upscaled_doge, raster_width, raster_height);
    rafgl_raster_bilinear_upsample(&upscaled_doge, &doge);


    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);

    int i;

    char tile_path[256];


        rafgl_spritesheet_init(&hero, "res/images/pixil-frame-0.png", 1, 4);
        rafgl_spritesheet_init(&enemy, "res/images/enemyShip.png", 1, 4);
             rafgl_spritesheet_init(&projectile, "res/images/pixil-frame-0 projcetile.png", 2, 1);
              rafgl_spritesheet_init(&explosion, "res/images/Eksplozija.png", 6, 1);
    rafgl_spritesheet_init(&enemy_projectile, "res/images/projectile.png", 2, 1);

    for(i = 0; i < NUMBER_OF_TILES; i++)
    {
      printf("%d\n",i);
        sprintf(tile_path, "res/space/svgset%d.png", i);
        rafgl_raster_load_from_image(&tiles[i], tile_path);

    }


    init_tilemap();
    rafgl_texture_init(&texture);
}


int pressed;
float location = 0;
float selector = 0;

int animation_running = 0;
int animation_frame = 0;
int animation_explosion=0;

int direction = 0;

int animation_frame_projectile=0;
int animation_frame_enemy_projectile=0;
int refresh=MAX_ENEMY;

int hero_pos_x = RASTER_WIDTH / 2;
int hero_pos_y = RASTER_HEIGHT /1.2;
int enemy_pos_x = RASTER_WIDTH/10 ;
int enemy_pos_y = 10;

int shoots_fired=4;
int hero_speed = 250;

int result=0;

int enemyspeed_speed = 100;
int hover_frames = 0;
int hover_frames_projectile = 0;
int hover_frames_enemy_projectile = 0;
int hover_frames_explosion=0;
int hover_frames_enemy_explosion=0;
int flagDirection=1;
int have_enemy=1;

int hero_healt=100;
int hit=0;
int speed_time=0;
int flag_enemy=1;
int br=0;
int b=0;
void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
           int i, gen = 5, radius = 10,enemy_gen=5;

        float angle, speed;

    if(game_data->is_lmb_down && game_data->is_rmb_down)
    {
        pressed = 1;
        location = rafgl_clampf(game_data->mouse_pos_y, 0, raster_height - 1);
        selector = 1.0f * location / raster_height;
    }
    else
    {
        pressed = 0;
    }

    animation_running = 1;

    //Pomeraj kamere
if(!camy) camy=81910;
else camy -=10;

if(hero_healt>=0){
    if(game_data->keys_down[RAFGL_KEY_W])
    {
        hero_pos_y = hero_pos_y - hero_speed * delta_time;
        direction = 2;
    }

    else if(game_data->keys_down[RAFGL_KEY_S])
    {
        if(hero_pos_y<RASTER_WIDTH-60*5){
        hero_pos_y = hero_pos_y + hero_speed * delta_time;
        }
        direction = 0;

    }
    else if(game_data->keys_down[RAFGL_KEY_A])
    {
         if(hero_pos_x>10){
        hero_pos_x = hero_pos_x - hero_speed * delta_time;
         }

        direction = 1;
    }

    else if(game_data->keys_down[RAFGL_KEY_D])
    {
         if(hero_pos_x<=raster_width-100){
        hero_pos_x = hero_pos_x + hero_speed * delta_time;
         }
        direction = 3;
    }
    else
    {
         direction = 2;
        animation_running = 0;
    }
    if(animation_running)
    {

        if(hover_frames == 0)
        {

            animation_frame = (animation_frame + 1) % 10;

            hover_frames = animation_frame;
        }
        else
        {

            hover_frames--;
        }

    }


        if(game_data->keys_pressed[RAFGL_KEY_SPACE])
        {

             for(i = 0; (i < MAX_PARTICLES) && gen; i++)
            {
                if(particles[i].life <= 0)
                {
                    particles[i].direction = 0;
                    particles[i].life = 200;
                    particles[i].x = hero_pos_x+hero.frame_width/4*1.45;
                    particles[i].y = hero_pos_y+hero.frame_height/2;


                    angle = 4.71239;

                    speed = 10;
                    particles[i].dx = cosf(angle) * speed;
                    particles[i].dy = sinf(angle) * speed;
                    gen--;

                }
            }

        }

}



//Interakcija tile sa herojom
if((tile_world[(camy+hero_pos_y+32)/TILE_SIZE][(camx+hero_pos_x+32)/TILE_SIZE]==4))
{
hero_speed=400;
speed_time=1000;

}

//Dok traje ubrzano vreme stvaramo efekte
if(speed_time){
    camy-=10;
        hero_speed=400;
              for(i = 0; (i < MAX_PARTICLES) && gen; i++)
        {

            if(sad_particles[i].life <= 0)
            {

               sad_particles[i].life = 100 * randf() + 400;
                sad_particles[i].x =rand()%raster_width;

                sad_particles[i].y = 0;
            //sad_particles[i].y = rand()%raster_height;
               angle = randf() * M_PI *  2.0f;
                speed = ( 0.3f + 0.7 * randf()) * radius;
                sad_particles[i].dx = 0;
                sad_particles[i].dy = sinf(angle) * speed;
                gen--;
            }
        }

        speed_time--;

}
 else {
        hero_speed=250;
 }
  update_sad_particles(delta_time);
   update_particles(delta_time);

   //Pravimo nase projektile
                for(int i = 0; i < MAX_PARTICLES; i++)
            {
                if(particles[i].life <= 0){
                            particles[i].x=-40;
                            particles[i].y=-40;
                }

                particles[i].life--;

                particles[i].x += particles[i].dx;
                particles[i].y += particles[i].dy;

                rafgl_raster_draw_spritesheet(&raster, &projectile, 1, 1, particles[i].x, particles[i].y);

            }
//Pravimo neprijatelje
   if(have_enemy){

            for(int i=0;i<MAX_ENEMY;i++){

                enemies[i].alive=1;
                enemies[i].shot=0;
                enemies[i].direction=rand() %(4 + 1 - 1)+1;
            enemies[i].enemy_pos_x=enemy_pos_x+(150*i);
            enemies[i].enemy_pos_y=enemy_pos_y;
            enemies[i].time=rand() %(350 + 1 - 250)+250;

            }

have_enemy=0;


        }
        //Provera kolizije ukoliko projektil pogodio neprijatelja ili nas
             for(int i = 0; i < MAX_PARTICLES; i++)

            {
                  for(int j=0;j<MAX_ENEMY;j++){


                        if( (abs(particles[i].x-enemies[j].enemy_pos_x*1.0)*1.0<=enemy.frame_width && particles[i].x>enemies[j].enemy_pos_x*1.0 )
                           && abs(particles[i].y- enemies[j].enemy_pos_y*1.0)*1.0<enemy.frame_height && enemies[j].shot==0)
                           {
                               particles[i].life=0;
                               enemies[j].shot=1;
                        enemies[j].alive=0;
                animation_explosion=1;
                            result++;
                    }
      if( (abs(enemy_particles[i].x-hero_pos_x*1.0)*1.0<=hero.frame_width && enemy_particles[i].x>hero_pos_x*1.0 )
                           && abs(enemy_particles[i].y-hero_pos_y*1.0)*1.0<hero.frame_height && hit==0)
                           {
                 enemy_particles[i].life=0;
hit=1;

                    }

            }
            }


    selected_x = rafgl_clampi((game_data->mouse_pos_x + camx) / TILE_SIZE, 0, WORLD_SIZE - 1);
    selected_y = rafgl_clampi((game_data->mouse_pos_y + camy) / TILE_SIZE, 0, WORLD_SIZE - 1);
    int x, y;

    float xn, yn;

    rafgl_pixel_rgb_t sampled, sampled2, resulting, resulting2;


    for(y = 0; y < raster_height; y++)
    {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++)
        {
            xn = 1.0f * x / raster_width;

            sampled = pixel_at_m(upscaled_doge, x, y);
            sampled2 = rafgl_point_sample(&doge, xn, yn);

            resulting = sampled;
            resulting2 = sampled2;

            resulting.rgba = rafgl_RGB(0, 0, 0);
            resulting = sampled;

            pixel_at_m(raster, x, y) = resulting;
            pixel_at_m(raster2, x, y) = resulting2;


            if(pressed && rafgl_distance1D(location, y) < 3 && x > raster_width - 15)
            {
                pixel_at_m(raster, x, y).rgba = rafgl_RGB(255, 0, 0);
            }

        }
    }



            if(hover_frames_projectile == 0)
            {
                animation_frame_projectile = (animation_frame_projectile + 1) % 2;
                hover_frames_projectile = 7;
            }
            else
            {
                hover_frames_projectile--;
            }

                        particle_t p;
            for(int i = 0; i < MAX_PARTICLES; i++)
            {

                p = particles[i];
                if(p.life <= 0) continue;


                rafgl_raster_draw_spritesheet(&raster, &projectile, animation_frame_projectile, particles[i].direction, particles[i].x, particles[i].y);
            }

    if(game_data->keys_pressed[RAFGL_KEY_KP_ADD])
    {
        tile_world[selected_y][selected_x]++;
        tile_world[selected_y][selected_x] %= NUMBER_OF_TILES;
    }

    render_tilemap(&raster);

    //U zavisnosti od brzine drugacija boja heroja
if(hero_healt  && hero_speed==250)
rafgl_raster_draw_spritesheet(&raster, &hero, animation_frame, direction, hero_pos_x, hero_pos_y);
else if(hero_healt && hero_speed==400){
rafgl_raster_draw_spritesheet_my_method(&raster, &hero, animation_frame, direction, hero_pos_x, hero_pos_y);
}
//Ako izgubimo promenimo boju ekrana i stvaramo partikle
else if(hero_healt<=0){
        hero_speed=0;
        flag_enemy=0;

        int brightness;

    rafgl_pixel_rgb_t sampled;
    rafgl_pixel_rgb_t result;
    for(int y = 0; y < raster_height; y++){
        for(int x = 0; x < raster_width; x++){

            sampled = pixel_at_m(raster,x,y);
  brightness = (sampled.r + sampled.g + sampled.b)/3;

            result.r = brightness;
            result.g = brightness;
            result.b = brightness;

            pixel_at_m(raster,x,y) = result;
        }
    }


          for(i = 0; (i < MAX_PARTICLES) && gen; i++)
        {
            if(particles[i].life <= 0)
            {
                particles[i].life = 100 * randf() + 100;
                particles[i].x = hero_pos_x+30;
                particles[i].y = hero_pos_y+30;

                angle = randf() * M_PI *  2.0f;
                speed = ( 0.3f + 0.7 * randf()) * radius;

                particles[i].dx = cosf(angle) * speed;
                particles[i].dy = sinf(angle) * speed;
                gen--;
            }
        }

rafgl_raster_draw_string(&raster,"GAME OVER",RASTER_WIDTH/2.5,RASTER_HEIGHT/2.5,rafgl_RGB(255,0,0),350);
}

//Ukoliko nam je ostao jos jedan hp igrica nas upozorava blinkovanjem crvene boje na poziciji naseg heroja
b++;
if(hero_healt==20 && b%2==0){
           rafgl_pixel_rgb_t result;
 for(y = hero_pos_y; y <hero_pos_y+hero.frame_height ; y++)
    {
        for(x = hero_pos_x; x < hero_pos_x+hero.frame_width; x++)
        {
 sampled = pixel_at_m(raster,x,y);

result.rgba = rafgl_RGB(244,0,0);

   pixel_at_m(raster,x,y) = result;
        }
    }

}
    update_sad_particles(delta_time);

   draw_sad_particles(&raster);
    if(animation_explosion)
    {

        if(hover_frames_explosion == 0)
        {

            animation_explosion = (animation_explosion + 1) % 10;

            hover_frames_explosion = 4;
        }
        else
        {

            hover_frames_explosion--;
        }

    }

    //Ukoliko smo pogodjeni smanji nam se hp i ekran nam pocrveni na sekund
    if(hit && hero_healt!=0){
   rafgl_pixel_rgb_t result;
 for(y = 0; y < raster_height; y++)
    {
        for(x = 0; x < raster_width; x++)
        {
 sampled = pixel_at_m(raster,x,y);

result.rgba = rafgl_RGB(255,0,0);

   pixel_at_m(raster,x,y) = result;
        }
    }
    hero_healt-=20;
hit=0;
}

      for(int i=0;i<MAX_ENEMY;i++){
//shoots_fired je brojac koji nam sluzi da neprijatelj ne puca non stop
            if(shoots_fired==0)
        {
            //kreiranje neprijateljskih projektila
                if(enemy_particles[i].life <= 0 && enemies[i].alive)
                {

                    enemy_particles[i].direction = 1;
                    enemy_particles[i].life = 700;
                    enemy_particles[i].x = enemies[i].enemy_pos_x+enemy.frame_width/4;
                    enemy_particles[i].y = enemies[i].enemy_pos_y+enemy.frame_height/2;
                    int  enemy_angle=3.14159;

                    int enemy_speed = 1;
                    enemy_particles[i].dy = cosf(enemy_angle) * enemy_speed;
                    enemy_particles[i].dx = sinf(enemy_angle) * enemy_speed;
                    enemy_gen--;
shoots_fired=4;
                }

        } else {
        shoots_fired--;
        }

            flagDirection++;
            if(enemies[i].direction==1){
                   enemies[i].enemy_pos_x-=enemyspeed_speed*delta_time;
            }else if(enemies[i].direction==3){
             enemies[i].enemy_pos_x+=enemyspeed_speed*delta_time;
            } else if(enemies[i].direction==2){
                 enemies[i].enemy_pos_y+=enemyspeed_speed*delta_time;
            }else if(enemies[i].direction==4){
                 enemies[i].enemy_pos_y-=enemyspeed_speed*delta_time;
            }
 for(int j=0;j<MAX_ENEMY;j++){
        if(j==i)continue;
//Provera ukoliko se neprijatelji sudare da promene pravac
 if( (abs( enemies[i].enemy_pos_x - enemies[j].enemy_pos_x) + abs(enemies[i].enemy_pos_y - enemies[j].enemy_pos_y)<=60)){

 if( enemies[i].enemy_pos_x <  enemies[j].enemy_pos_x){
    enemies[i].direction=1;

 } else if (enemies[i].enemy_pos_x >  enemies[j].enemy_pos_x) {
  enemies[i].direction=3;
 }
else  if( enemies[i].enemy_pos_y <  enemies[j].enemy_pos_y){
    enemies[i].direction=4;

 } else {
  enemies[i].direction=2;
 }

 }

 }
 //Da neprijatelj ne izadje sa ekrana
            if(  enemies[i].enemy_pos_y>RASTER_HEIGHT/3
               ||enemies[i].enemy_pos_y<0  ||
               enemies[i].enemy_pos_x<10 || enemies[i].enemy_pos_x>raster_width-100 || flagDirection>=enemies[i].time){
                flagDirection=0;

            if(enemies[i].enemy_pos_y>RASTER_HEIGHT/3 )    enemies[i].enemy_pos_y-=10;
            if(enemies[i].enemy_pos_x>raster_width-100 )  enemies[i].enemy_pos_x-=10;
            if(enemies[i].enemy_pos_y<0  ) enemies[i].enemy_pos_y+=10;
            if( enemies[i].enemy_pos_x<10 )   enemies[i].enemy_pos_x+=10;

              enemies[i].direction=rand()%(4+1-1)+1;
              flagDirection=0;
            }


        if(enemies[i].enemy_pos_x<10 || enemies[i].enemy_pos_x>raster_width-100 && flagDirection==0){
                flagDirection=1;
        }

                if(enemies[i].alive && enemies[i].shot==0){
                        rafgl_raster_draw_spritesheet(&raster, &enemy, animation_frame, 1, enemies[i].enemy_pos_x, enemies[i].enemy_pos_y);

                }

                else if(enemies[i].shot==1){
            rafgl_raster_draw_spritesheet(&raster, &explosion, animation_explosion, 0, enemies[i].enemy_pos_x, enemies[i].enemy_pos_y);
            enemies[i].shot=2;

                }

if(enemies[i].shot==2) refresh--;
else refresh=MAX_ENEMY;
         for(int i = 0; i < MAX_PARTICLES; i++)
            {
                if(enemy_particles[i].life <= 0){
                            enemy_particles[i].x=+40;
                            enemy_particles[i].y=-40;
                }

                enemy_particles[i].life--;
                enemy_particles[i].y -= enemy_particles[i].dy;

                rafgl_raster_draw_spritesheet(&raster, &enemy_projectile, 0, 0, enemy_particles[i].x, enemy_particles[i].y);


            }

            }
//Ukoliko nemamo vise neprijatelja napravi ih
if(refresh<0 && flag_enemy){
            have_enemy=1;
            refresh=MAX_ENEMY;
        }

draw_sad_particles(&raster);

    if(game_data->keys_pressed[RAFGL_KEY_S] && game_data->keys_down[RAFGL_KEY_LEFT_SHIFT])
    {

        sprintf(save_file, "save%d.png", save_file_no++);
        rafgl_raster_save_to_png(&raster, save_file);
    }


}


void main_state_render(GLFWwindow *window, void *args)
{
    rafgl_texture_load_from_raster(&texture, &raster);
    rafgl_texture_show(&texture, 0);
}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_raster_cleanup(&raster2);
    rafgl_texture_cleanup(&texture);

}
