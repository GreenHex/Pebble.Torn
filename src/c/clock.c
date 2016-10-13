#include <pebble.h>
#include "global.h"
#include "clock.h"

#define FONT_HEIGHT 58

static Layer *window_layer = 0;
static TextLayer *digital_clock_text_layer = 0;
static GFont large_digital_font = 0;
static tm tm_time;

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global
  
  layer_mark_dirty( text_layer_get_layer( digital_clock_text_layer ) );
  // if (DEBUG) APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %0d:%0d:%0d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
}

static void digital_clock_text_layer_update_proc( Layer *layer, GContext *ctx ) {
  // uses global tm_time
  
  static char str_time[] = "xx:xx";
      
  strftime( str_time, sizeof( str_time ), ( clock_is_24h_style() ?  "%H:%M" : "%I:%M" ), &tm_time );
  
  if(str_time[0] == '0') memmove( &str_time[0], &str_time[1], sizeof( str_time ) - 1 );
  
  // if (DEBUG) APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: digital_clock_text_layer_update_proc(): %s", str_time );
  
  GRect layer_bounds = layer_get_bounds( layer );
  GRect text_bounds = layer_bounds;
  text_bounds.origin.y = ( text_bounds.size.h - FONT_HEIGHT ) / 2;
  graphics_context_set_fill_color( ctx, GColorBlack );
  graphics_fill_rect( ctx, layer_bounds, 0, GCornersAll );
  graphics_context_set_text_color( ctx, GColorWhite );
  graphics_draw_text( ctx, str_time, large_digital_font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, 0 );
}

static void prv_unobstructed_change( AnimationProgress progress, void *window_root_layer ) {
  
  // GRect full_bounds = layer_get_bounds( window_root_layer );
  GRect unobstructed_bounds = layer_get_unobstructed_bounds( window_root_layer );
  layer_set_frame( text_layer_get_layer( digital_clock_text_layer ), unobstructed_bounds );
}

static void prv_unobstructed_did_change( void *context ) {

}

void clock_init( Window *window ) {
  
  window_layer = window_get_root_layer( window );
  GRect window_bounds = layer_get_bounds( window_layer );
  GRect clock_layer_bounds = window_bounds; 
  
  digital_clock_text_layer = text_layer_create( clock_layer_bounds );
  layer_add_child( window_layer, text_layer_get_layer( digital_clock_text_layer ) );
  layer_set_update_proc( text_layer_get_layer( digital_clock_text_layer ), digital_clock_text_layer_update_proc );
  layer_set_hidden( text_layer_get_layer( digital_clock_text_layer ), false );
  large_digital_font = fonts_get_system_font( FONT_KEY_BITHAM_42_LIGHT );
  
  UnobstructedAreaHandlers handler = {
    .change = prv_unobstructed_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe( handler, window_layer );
  
  // subscriptions
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
}

void clock_deinit( void ) {
  tick_timer_service_unsubscribe();
  text_layer_destroy( digital_clock_text_layer );
}