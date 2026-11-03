#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void draw_citron(cairo_t *cr, double x, double y, double size) {
    const double s=size/28.0;
    cairo_save(cr); cairo_translate(cr,x,y); cairo_scale(cr,s,s);
    cairo_set_source_rgba(cr,0,0,0,.25); cairo_arc(cr,14.5,16.1,10.9,0,2*M_PI); cairo_fill(cr);
    cairo_move_to(cr,11.1,6.1); cairo_curve_to(cr,5.8,6.2,2.2,9.2,2.8,13.5); cairo_curve_to(cr,3.3,20,8.8,23,14.2,21.6); cairo_curve_to(cr,20,20.1,24.7,15.1,22,10.3); cairo_curve_to(cr,20,6.8,15,5,11.1,6.1); cairo_close_path(cr);
    cairo_set_source_rgb(cr,.82,.88,.42); cairo_fill_preserve(cr); cairo_set_source_rgba(cr,1,1,.75,.85); cairo_set_line_width(cr,.7); cairo_stroke(cr);
    cairo_move_to(cr,11,6.3); cairo_curve_to(cr,10.5,3.4,13,1.1,17.7,1); cairo_curve_to(cr,17.2,4.7,14.8,6.6,11,6.3); cairo_close_path(cr); cairo_set_source_rgb(cr,.46,.59,.27); cairo_fill(cr);
    cairo_move_to(cr,5.7,12.6); cairo_curve_to(cr,6.8,10.2,8.8,9,10.7,8.7); cairo_set_source_rgba(cr,1,1,.83,.8); cairo_set_line_width(cr,.9); cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND); cairo_stroke(cr);
    cairo_restore(cr);
}

static void save_mark(const char *path) {
    cairo_surface_t *s=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,256,256); cairo_t *cr=cairo_create(s);
    cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE); cairo_set_source_rgba(cr,0,0,0,0); cairo_paint(cr); cairo_set_operator(cr,CAIRO_OPERATOR_OVER);
    draw_citron(cr,4,4,248); cairo_destroy(cr); cairo_surface_write_to_png(s,path); cairo_surface_destroy(s);
}

static void save_splash(const char *path,int w,int h) {
    cairo_surface_t *s=cairo_image_surface_create(CAIRO_FORMAT_RGB24,w,h); cairo_t *cr=cairo_create(s);
    cairo_pattern_t *p=cairo_pattern_create_linear(0,0,w,h);
    cairo_pattern_add_color_stop_rgb(p,0,.055,.075,.105); cairo_pattern_add_color_stop_rgb(p,.52,.105,.155,.16); cairo_pattern_add_color_stop_rgb(p,1,.145,.135,.105); cairo_set_source(cr,p); cairo_paint(cr); cairo_pattern_destroy(p);
    cairo_pattern_t *g=cairo_pattern_create_radial(w*.5,h*.31,2,w*.5,h*.31,w*.47); cairo_pattern_add_color_stop_rgba(g,0,.55,.68,.3,.19); cairo_pattern_add_color_stop_rgba(g,1,.1,.14,.16,0); cairo_set_source(cr,g); cairo_paint(cr); cairo_pattern_destroy(g);
    cairo_set_source_rgba(cr,1,1,1,.025); cairo_set_line_width(cr,1); for(int x=0;x<w;x+=56){cairo_move_to(cr,x,0);cairo_line_to(cr,x,h);} for(int y=0;y<h;y+=56){cairo_move_to(cr,0,y);cairo_line_to(cr,w,y);} cairo_stroke(cr);
    double mark=fmin(w*.20,h*.27); draw_citron(cr,(w-mark)/2,h*.07,mark);
    cairo_select_font_face(cr,"DejaVu Sans",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD); cairo_set_font_size(cr,fmin(w*.056,38)); cairo_text_extents_t ext; cairo_text_extents(cr,"CITRON OS",&ext); cairo_set_source_rgb(cr,.94,.95,.89); cairo_move_to(cr,(w-ext.width)/2,h*.45); cairo_show_text(cr,"CITRON OS");
    cairo_select_font_face(cr,"DejaVu Sans",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL); cairo_set_font_size(cr,fmin(w*.019,14)); cairo_text_extents(cr,"Démarrage du système",&ext); cairo_set_source_rgba(cr,.77,.81,.77,.85); cairo_move_to(cr,(w-ext.width)/2,h*.50); cairo_show_text(cr,"Démarrage du système");
    cairo_destroy(cr); cairo_surface_write_to_png(s,path); cairo_surface_destroy(s);
}

static void rounded_rect(cairo_t *cr,double x,double y,double w,double h,double r){cairo_new_sub_path(cr);cairo_arc(cr,x+w-r,y+r,r,-M_PI/2,0);cairo_arc(cr,x+w-r,y+h-r,r,0,M_PI/2);cairo_arc(cr,x+r,y+h-r,r,M_PI/2,M_PI);cairo_arc(cr,x+r,y+r,r,M_PI,3*M_PI/2);cairo_close_path(cr);}
static void save_loading_assets(void){
    cairo_surface_t*s=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,640,22);cairo_t*cr=cairo_create(s);cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);cairo_set_source_rgba(cr,0,0,0,0);cairo_paint(cr);cairo_set_operator(cr,CAIRO_OPERATOR_OVER);rounded_rect(cr,1,3,638,16,8);cairo_set_source_rgba(cr,.88,.91,.82,.12);cairo_fill_preserve(cr);cairo_set_source_rgba(cr,.88,.91,.82,.28);cairo_set_line_width(cr,1);cairo_stroke(cr);cairo_destroy(cr);cairo_surface_write_to_png(s,"progress-track.png");cairo_surface_destroy(s);
    s=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,640,22);cr=cairo_create(s);cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);cairo_set_source_rgba(cr,0,0,0,0);cairo_paint(cr);cairo_set_operator(cr,CAIRO_OPERATOR_OVER);rounded_rect(cr,0,5,640,12,6);cairo_pattern_t*p=cairo_pattern_create_linear(0,0,640,0);cairo_pattern_add_color_stop_rgb(p,0,.78,.85,.37);cairo_pattern_add_color_stop_rgb(p,1,.87,.92,.53);cairo_set_source(cr,p);cairo_fill(cr);cairo_pattern_destroy(p);cairo_destroy(cr);cairo_surface_write_to_png(s,"progress-fill.png");cairo_surface_destroy(s);
    s=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,20,20);cr=cairo_create(s);cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);cairo_set_source_rgba(cr,0,0,0,0);cairo_paint(cr);cairo_set_operator(cr,CAIRO_OPERATOR_OVER);cairo_arc(cr,10,10,4.5,0,2*M_PI);cairo_set_source_rgba(cr,.84,.9,.47,1);cairo_fill(cr);cairo_destroy(cr);cairo_surface_write_to_png(s,"progress-dot.png");cairo_surface_destroy(s);
}

int main(int argc,char **argv){if(argc==2&&!strcmp(argv[1],"--mark")){save_mark("citron-mark.png");return 0;}if(argc==2&&!strcmp(argv[1],"--loading-assets")){save_loading_assets();return 0;}if(argc!=4){fprintf(stderr,"usage: %s OUTPUT.png WIDTH HEIGHT\n",argv[0]);return 2;}save_splash(argv[1],atoi(argv[2]),atoi(argv[3]));return 0;}
