#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *desktop;
static GtkWidget *start_button;
static GtkWidget *clock_label;
static GtkWidget *task_clock_label;

static const char *css =
"* { font-family: 'DejaVu Sans', sans-serif; }"
".desktop { color:#f4f5ef; background:#121720; }"
".topbar { background:rgba(17,23,32,.88); border-bottom:1px solid rgba(255,255,255,.08); padding:0 23px; }"
".brand { color:#f4f5ef; font-size:14px; font-weight:bold; padding:0; }"
".brand-os { color:#abb0b0; font-weight:normal; }"
".muted { color:#a6acab; font-size:11px; }"
".tiny { color:#dfe2da; font-size:11px; }"
".taskbar { background:rgba(20,26,32,.92); border-top:1px solid rgba(255,255,255,.10); padding:0 14px; }"
".shell-button { color:#e5e8df; background:transparent; border:0; border-radius:9px; padding:7px 9px; }"
".shell-button:hover { background:rgba(255,255,255,.12); }"
".desktop-shortcut { color:#e3e4df; background:transparent; border:1px solid transparent; border-radius:10px; padding:8px 4px; }"
".desktop-shortcut:hover { background:rgba(255,255,255,.09); border-color:rgba(255,255,255,.12); }"
".welcome { background:rgba(255,255,255,.055); border:1px solid rgba(255,255,255,.10); border-radius:16px; padding:19px; }"
".welcome-kicker { color:#cbd58e; font-size:10px; letter-spacing:1.6px; }"
".welcome-time { color:#f3f4ed; font-size:44px; font-weight:300; }"
".welcome-sub { color:#a7afaa; font-size:12px; }"
".welcome-foot { color:#c3c9be; font-size:10px; border-top:1px solid rgba(255,255,255,.10); padding-top:10px; }"
".popover { background:#20282e; color:#f4f5ef; border:1px solid rgba(255,255,255,.17); border-radius:14px; }"
".app-tile { color:#e4e8e0; background:transparent; border:0; border-radius:9px; padding:10px 5px; }"
".app-tile:hover { background:rgba(255,255,255,.10); }"
".window { background:#f5f5f1; color:#30352c; border-radius:12px; }"
".window-head { background:#f5f5f1; padding:10px 14px; border-bottom:1px solid #e6e7e0; }"
".window-title { color:#30352c; font-size:12px; font-weight:bold; }"
".window-content { background:#fbfbf8; color:#30352c; padding:18px; }"
".window-content textview, .window-content entry { background:#fff; color:#30352c; }"
".window-content button { padding:7px 11px; }";

static gboolean citron_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GtkAllocation a; gtk_widget_get_allocation(widget, &a);
    double s = MIN(a.width, a.height) / 28.0;
    cairo_save(cr); cairo_translate(cr, (a.width-28*s)/2, (a.height-28*s)/2); cairo_scale(cr,s,s);
    cairo_set_source_rgba(cr,0,0,0,.22); cairo_arc(cr,14,16,11,0,2*M_PI); cairo_fill(cr);
    cairo_move_to(cr,11.1,6.1); cairo_curve_to(cr,5.8,6.2,2.2,9.2,2.8,13.5); cairo_curve_to(cr,3.3,20,8.8,23,14.2,21.6); cairo_curve_to(cr,20,20.1,24.7,15.1,22,10.3); cairo_curve_to(cr,20,6.8,15,5,11.1,6.1); cairo_close_path(cr);
    cairo_set_source_rgb(cr,.82,.88,.42); cairo_fill_preserve(cr); cairo_set_source_rgba(cr,1,1,.75,.85); cairo_set_line_width(cr,.7); cairo_stroke(cr);
    cairo_move_to(cr,11,6.3); cairo_curve_to(cr,10.5,3.4,13,1.1,17.7,1); cairo_curve_to(cr,17.2,4.7,14.8,6.6,11,6.3); cairo_close_path(cr); cairo_set_source_rgb(cr,.46,.59,.27); cairo_fill(cr);
    cairo_move_to(cr,5.7,12.6); cairo_curve_to(cr,6.8,10.2,8.8,9,10.7,8.7); cairo_set_source_rgba(cr,1,1,.83,.8); cairo_set_line_width(cr,.9); cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND); cairo_stroke(cr);
    cairo_restore(cr); return FALSE;
}

static GtkWidget *citron_icon(int size) {
    GtkWidget *w=gtk_drawing_area_new(); gtk_widget_set_size_request(w,size,size); g_signal_connect(w,"draw",G_CALLBACK(citron_draw),NULL); return w;
}

static gboolean wallpaper_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GtkAllocation a; gtk_widget_get_allocation(widget,&a);
    cairo_pattern_t *p=cairo_pattern_create_linear(0,0,a.width,a.height);
    cairo_pattern_add_color_stop_rgb(p,0,.075,.09,.12); cairo_pattern_add_color_stop_rgb(p,.55,.105,.145,.16); cairo_pattern_add_color_stop_rgb(p,1,.13,.12,.105);
    cairo_set_source(cr,p); cairo_paint(cr); cairo_pattern_destroy(p);
    cairo_pattern_t *g=cairo_pattern_create_radial(a.width*.67,a.height*.4,2,a.width*.67,a.height*.4,a.width*.48);
    cairo_pattern_add_color_stop_rgba(g,0,.43,.60,.44,.16); cairo_pattern_add_color_stop_rgba(g,1,.1,.13,.15,0); cairo_set_source(cr,g); cairo_paint(cr); cairo_pattern_destroy(g);
    cairo_set_source_rgba(cr,1,1,1,.018); cairo_set_line_width(cr,1);
    for(int x=0;x<a.width;x+=56){cairo_move_to(cr,x,0);cairo_line_to(cr,x,a.height);} for(int y=0;y<a.height;y+=56){cairo_move_to(cr,0,y);cairo_line_to(cr,a.width,y);} cairo_stroke(cr);
    return FALSE;
}

static GtkWidget *label(const char *s, const char *klass) { GtkWidget *w=gtk_label_new(s); if(klass) gtk_style_context_add_class(gtk_widget_get_style_context(w),klass); return w; }
static void launch_command(const char *cmd) { GError *e=NULL; if(!g_spawn_command_line_async(cmd,&e)){GtkWidget*d=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Impossible de lancer l’application");gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(d),"%s",e?e->message:"Erreur inconnue");gtk_dialog_run(GTK_DIALOG(d));gtk_widget_destroy(d);if(e)g_error_free(e);} }

static GtkWidget *make_dialog(const char *title, int width, int height, GtkWidget **body_out) {
    GtkWidget *w=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_title(GTK_WINDOW(w),title); gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(desktop)); gtk_window_set_modal(GTK_WINDOW(w),TRUE); gtk_window_set_position(GTK_WINDOW(w),GTK_WIN_POS_CENTER_ON_PARENT); gtk_window_set_default_size(GTK_WINDOW(w),width,height);
    gtk_style_context_add_class(gtk_widget_get_style_context(w),"window"); GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,0); gtk_container_add(GTK_CONTAINER(w),box);
    GtkWidget *head=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8); gtk_style_context_add_class(gtk_widget_get_style_context(head),"window-head"); GtkWidget *title_l=label(title,"window-title"); gtk_box_pack_start(GTK_BOX(head),title_l,TRUE,TRUE,0); GtkWidget *close=gtk_button_new_with_label("×"); gtk_button_set_relief(GTK_BUTTON(close),GTK_RELIEF_NONE); g_signal_connect_swapped(close,"clicked",G_CALLBACK(gtk_widget_destroy),w); gtk_box_pack_end(GTK_BOX(head),close,FALSE,FALSE,0); gtk_box_pack_start(GTK_BOX(box),head,FALSE,FALSE,0);
    GtkWidget *body=gtk_box_new(GTK_ORIENTATION_VERTICAL,12); gtk_style_context_add_class(gtk_widget_get_style_context(body),"window-content"); gtk_box_pack_start(GTK_BOX(box),body,TRUE,TRUE,0); if(body_out)*body_out=body; return w;
}

static void open_files(void) {
    GtkWidget *d=gtk_file_chooser_dialog_new("Fichiers — Citron OS",GTK_WINDOW(desktop),GTK_FILE_CHOOSER_ACTION_OPEN,"Annuler",GTK_RESPONSE_CANCEL,"Ouvrir",GTK_RESPONSE_ACCEPT,NULL);
    gtk_window_set_default_size(GTK_WINDOW(d),780,500); gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(d),TRUE);
    if(gtk_dialog_run(GTK_DIALOG(d))==GTK_RESPONSE_ACCEPT){char*path=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));if(path){char*uri=g_filename_to_uri(path,NULL,NULL);if(uri){g_app_info_launch_default_for_uri(uri,NULL,NULL);g_free(uri);}g_free(path);}}
    gtk_widget_destroy(d);
}

static void save_note(GtkWidget *view, GtkWidget *status) {
    GtkTextBuffer *b=gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)); GtkTextIter start,end; gtk_text_buffer_get_bounds(b,&start,&end); gchar *text=gtk_text_buffer_get_text(b,&start,&end,FALSE);
    gchar *dir=g_build_filename(g_get_home_dir(),"Documents",NULL); g_mkdir_with_parents(dir,0755); gchar *path=g_build_filename(dir,"Notes-Citron.txt",NULL); GError *e=NULL;
    if(g_file_set_contents(path,text,-1,&e)) gtk_label_set_text(GTK_LABEL(status),"Note enregistrée dans Documents/Notes-Citron.txt"); else {gtk_label_set_text(GTK_LABEL(status),e->message);g_error_free(e);} g_free(path);g_free(dir);g_free(text);
}
static void save_note_clicked(GtkButton *button, gpointer data) { save_note(g_object_get_data(G_OBJECT(button),"note-view"),g_object_get_data(G_OBJECT(button),"note-status")); }

static void open_notes(void) {
    GtkWidget *body,*w=make_dialog("Notes",700,510,&body); GtkWidget *title=gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(title),"Titre de la note"); gtk_entry_set_text(GTK_ENTRY(title),"Mes idées"); gtk_box_pack_start(GTK_BOX(body),title,FALSE,FALSE,0);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL); gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC); GtkWidget *view=gtk_text_view_new(); gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view),GTK_WRAP_WORD_CHAR); gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view),14); gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view),12); gtk_container_add(GTK_CONTAINER(scroll),view); gtk_box_pack_start(GTK_BOX(body),scroll,TRUE,TRUE,0);
    gchar *path=g_build_filename(g_get_home_dir(),"Documents","Notes-Citron.txt",NULL),*contents=NULL; if(g_file_get_contents(path,&contents,NULL,NULL)){gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)),contents,-1);g_free(contents);}g_free(path);
    GtkWidget *row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10),*status=label("Notes enregistrées localement","muted"),*save=gtk_button_new_with_label("Enregistrer");
    gtk_box_pack_start(GTK_BOX(row),status,TRUE,TRUE,0); gtk_box_pack_end(GTK_BOX(row),save,FALSE,FALSE,0); gtk_box_pack_start(GTK_BOX(body),row,FALSE,FALSE,0);
    g_object_set_data(G_OBJECT(save),"note-view",view); g_object_set_data(G_OBJECT(save),"note-status",status);
    g_signal_connect(save,"clicked",G_CALLBACK(save_note_clicked),NULL);
    gtk_widget_show_all(w);
}

/* Small recursive-descent evaluator for calculator expressions. */
static const char *expr_p; static gboolean expr_bad;
static void ws(void){while(g_ascii_isspace(*expr_p))expr_p++;}
static double parse_sum(void);
static double parse_atom(void){ws();if(*expr_p=='('){expr_p++;double v=parse_sum();ws();if(*expr_p==')')expr_p++;else expr_bad=TRUE;return v;}char*end;double v=g_ascii_strtod(expr_p,&end);if(end==expr_p){expr_bad=TRUE;return 0;}expr_p=end;return v;}
static double parse_product(void){double v=parse_atom();for(;;){ws();char op=*expr_p;if(op!='*'&&op!='/')break;expr_p++;double r=parse_atom();if(op=='/'&&r==0){expr_bad=TRUE;return 0;}v=op=='*'?v*r:v/r;}return v;}
static double parse_sum(void){double v=parse_product();for(;;){ws();char op=*expr_p;if(op!='+'&&op!='-')break;expr_p++;double r=parse_product();v=op=='+'?v+r:v-r;}return v;}
static void calc_key(GtkButton *b,gpointer entry){const char*k=gtk_button_get_label(b);if(!strcmp(k,"=")){const char*s=gtk_entry_get_text(GTK_ENTRY(entry));expr_p=s;expr_bad=FALSE;double v=parse_sum();ws();if(expr_bad||*expr_p)gtk_entry_set_text(GTK_ENTRY(entry),"Erreur");else{char out[80];g_snprintf(out,sizeof(out),"%.10g",v);gtk_entry_set_text(GTK_ENTRY(entry),out);}return;}if(!strcmp(k,"AC")){gtk_entry_set_text(GTK_ENTRY(entry),"");return;}if(!strcmp(k,"⌫")){const char*s=gtk_entry_get_text(GTK_ENTRY(entry));int n=g_utf8_strlen(s,-1);if(n>0){gchar*t=g_utf8_substring(s,0,n-1);gtk_entry_set_text(GTK_ENTRY(entry),t);g_free(t);}return;}const char*s=gtk_entry_get_text(GTK_ENTRY(entry));gchar*t=g_strconcat(s,k,NULL);gtk_entry_set_text(GTK_ENTRY(entry),t);g_free(t);}

static void open_calculator(void){GtkWidget*body,*w=make_dialog("Calculatrice",330,420,&body);GtkWidget*entry=gtk_entry_new();gtk_entry_set_alignment(GTK_ENTRY(entry),1);gtk_entry_set_placeholder_text(GTK_ENTRY(entry),"0");gtk_widget_set_name(entry,"calc-display");gtk_box_pack_start(GTK_BOX(body),entry,FALSE,FALSE,0);GtkWidget*grid=gtk_grid_new();gtk_grid_set_row_spacing(GTK_GRID(grid),7);gtk_grid_set_column_spacing(GTK_GRID(grid),7);gtk_box_pack_start(GTK_BOX(body),grid,TRUE,TRUE,0);const char*keys[]={"AC","⌫","(",")","7","8","9","/","4","5","6","*","1","2","3","-","0",".","=","+"};for(int i=0;i<20;i++){GtkWidget*b=gtk_button_new_with_label(keys[i]);gtk_widget_set_size_request(b,58,49);gtk_grid_attach(GTK_GRID(grid),b,i%4,i/4,1,1);g_signal_connect(b,"clicked",G_CALLBACK(calc_key),entry);}gtk_widget_show_all(w);}

static void open_settings(void){GtkWidget*body,*w=make_dialog("Réglages",520,430,&body);gtk_box_pack_start(GTK_BOX(body),label("Personnalisez votre espace Citron.","muted"),FALSE,FALSE,0);const char*items[]={"Apparence — Thème et couleurs","Réseau et Internet — Connexion active","Notifications — Alertes système","À propos — Citron OS 1.0 · Debian Linux"};for(int i=0;i<4;i++){GtkWidget*r=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);GtkWidget*l=label(items[i],"tiny");gtk_box_pack_start(GTK_BOX(r),l,TRUE,TRUE,7);gtk_box_pack_start(GTK_BOX(body),r,FALSE,FALSE,8);GtkWidget*sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);gtk_box_pack_start(GTK_BOX(body),sep,FALSE,FALSE,0);}gtk_widget_show_all(w);}

static void launch_app(const char *id){if(!strcmp(id,"Fichiers"))open_files();else if(!strcmp(id,"Notes"))open_notes();else if(!strcmp(id,"Calculatrice"))open_calculator();else if(!strcmp(id,"Réglages"))open_settings();else if(!strcmp(id,"Internet"))launch_command("firefox-esr");else if(!strcmp(id,"Terminal"))launch_command("xfce4-terminal");}
static void app_clicked(GtkButton*b,gpointer d){launch_app(g_object_get_data(G_OBJECT(b),"app-id"));}

static GtkWidget *app_tile(const char *name,const char *glyph){GtkWidget*b=gtk_button_new(),*v=gtk_box_new(GTK_ORIENTATION_VERTICAL,6),*ico=label(glyph,NULL),*txt=label(name,NULL);gtk_label_set_markup(GTK_LABEL(ico),g_strdup_printf("<span size='xx-large' foreground='#d5df89'>%s</span>",glyph));gtk_label_set_markup(GTK_LABEL(txt),g_strdup_printf("<span size='small'>%s</span>",name));gtk_box_pack_start(GTK_BOX(v),ico,FALSE,FALSE,0);gtk_box_pack_start(GTK_BOX(v),txt,FALSE,FALSE,0);gtk_container_add(GTK_CONTAINER(b),v);gtk_style_context_add_class(gtk_widget_get_style_context(b),"app-tile");g_object_set_data_full(G_OBJECT(b),"app-id",g_strdup(name),g_free);g_signal_connect(b,"clicked",G_CALLBACK(app_clicked),NULL);return b;}

static void update_clock(void){time_t now=time(NULL);struct tm*t=localtime(&now);char s[80];strftime(s,sizeof(s),"%H:%M",t);gtk_label_set_text(GTK_LABEL(clock_label),s);gtk_label_set_text(GTK_LABEL(task_clock_label),s);GtkWidget*w=g_object_get_data(G_OBJECT(desktop),"welcome-time");if(w)gtk_label_set_text(GTK_LABEL(w),s);}
static gboolean clock_tick(gpointer d){update_clock();return TRUE;}

static void pin_clicked(GtkButton*b,gpointer d){launch_app((const char*)d);}
static GtkWidget *shortcut(const char*name,const char*glyph){GtkWidget*b=gtk_button_new(),*v=gtk_box_new(GTK_ORIENTATION_VERTICAL,7),*ico=label(glyph,NULL),*txt=label(name,NULL);gtk_label_set_markup(GTK_LABEL(ico),g_strdup_printf("<span size='xx-large' foreground='#d8e48a'>%s</span>",glyph));gtk_label_set_markup(GTK_LABEL(txt),g_strdup_printf("<span size='small' foreground='#e3e4df'>%s</span>",name));gtk_box_pack_start(GTK_BOX(v),ico,FALSE,FALSE,0);gtk_box_pack_start(GTK_BOX(v),txt,FALSE,FALSE,0);gtk_container_add(GTK_CONTAINER(b),v);gtk_style_context_add_class(gtk_widget_get_style_context(b),"desktop-shortcut");g_object_set_data_full(G_OBJECT(b),"app-id",g_strdup(name),g_free);g_signal_connect(b,"clicked",G_CALLBACK(app_clicked),NULL);return b;}

int main(int argc,char**argv){gtk_init(&argc,&argv);GtkCssProvider*provider=gtk_css_provider_new();gtk_css_provider_load_from_data(provider,css,-1,NULL);gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    desktop=gtk_window_new(GTK_WINDOW_TOPLEVEL);gtk_window_set_title(GTK_WINDOW(desktop),"Citron OS");gtk_window_set_decorated(GTK_WINDOW(desktop),FALSE);gtk_window_fullscreen(GTK_WINDOW(desktop));gtk_widget_set_name(desktop,"citron-desktop");gtk_style_context_add_class(gtk_widget_get_style_context(desktop),"desktop");g_signal_connect(desktop,"destroy",G_CALLBACK(gtk_main_quit),NULL);
    GtkWidget*overlay=gtk_overlay_new();gtk_container_add(GTK_CONTAINER(desktop),overlay);GtkWidget*wall=gtk_drawing_area_new();g_signal_connect(wall,"draw",G_CALLBACK(wallpaper_draw),NULL);gtk_container_add(GTK_CONTAINER(overlay),wall);
    GtkWidget*root=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);gtk_overlay_add_overlay(GTK_OVERLAY(overlay),root);
    GtkWidget*top=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,15);gtk_widget_set_size_request(top,-1,50);gtk_style_context_add_class(gtk_widget_get_style_context(top),"topbar");gtk_box_pack_start(GTK_BOX(root),top,FALSE,FALSE,0);
    GtkWidget*brand=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);gtk_box_pack_start(GTK_BOX(brand),citron_icon(27),FALSE,FALSE,0);GtkWidget*brandtxt=gtk_label_new(NULL);gtk_label_set_markup(GTK_LABEL(brandtxt),"<b>citron</b><span foreground='#abb0b0'> OS</span>");gtk_box_pack_start(GTK_BOX(brand),brandtxt,FALSE,FALSE,0);GtkWidget*brandbtn=gtk_button_new();gtk_container_add(GTK_CONTAINER(brandbtn),brand);gtk_style_context_add_class(gtk_widget_get_style_context(brandbtn),"brand");gtk_style_context_add_class(gtk_widget_get_style_context(brandbtn),"shell-button");gtk_box_pack_start(GTK_BOX(top),brandbtn,FALSE,FALSE,0);
    GtkWidget*status=label("●  Tout est à sa place   /   Bureau 01","muted");gtk_box_pack_start(GTK_BOX(top),status,TRUE,TRUE,0);GtkWidget*weather=label("☀  18°","tiny");gtk_box_pack_end(GTK_BOX(top),weather,FALSE,FALSE,5);clock_label=label("--:--","tiny");gtk_box_pack_end(GTK_BOX(top),clock_label,FALSE,FALSE,8);GtkWidget*avatar=gtk_button_new_with_label("E");gtk_style_context_add_class(gtk_widget_get_style_context(avatar),"shell-button");gtk_box_pack_end(GTK_BOX(top),avatar,FALSE,FALSE,0);
    GtkWidget*area=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);gtk_box_pack_start(GTK_BOX(root),area,TRUE,TRUE,0);
    GtkWidget*left=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);gtk_widget_set_halign(left,GTK_ALIGN_START);gtk_widget_set_valign(left,GTK_ALIGN_START);gtk_widget_set_margin_start(left,24);gtk_widget_set_margin_top(left,26);gtk_box_pack_start(GTK_BOX(area),left,FALSE,FALSE,0);
    const char*sn[]={"Fichiers","Notes","Internet","Réglages"};const char*sg[]={"▰","▤","◎","⚙"};for(int i=0;i<4;i++)gtk_box_pack_start(GTK_BOX(left),shortcut(sn[i],sg[i]),FALSE,FALSE,0);
    GtkWidget*spacer=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);gtk_box_pack_start(GTK_BOX(area),spacer,TRUE,TRUE,0);
    GtkWidget*card=gtk_box_new(GTK_ORIENTATION_VERTICAL,9);gtk_style_context_add_class(gtk_widget_get_style_context(card),"welcome");gtk_widget_set_size_request(card,280,-1);gtk_widget_set_halign(card,GTK_ALIGN_END);gtk_widget_set_valign(card,GTK_ALIGN_CENTER);gtk_widget_set_margin_end(card,70);gtk_box_pack_start(GTK_BOX(area),card,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(card),label("✳   MARDI · VOTRE ESPACE","welcome-kicker"),FALSE,FALSE,0);GtkWidget*big=label("--:--","welcome-time");gtk_box_pack_start(GTK_BOX(card),big,FALSE,FALSE,0);g_object_set_data(G_OBJECT(desktop),"welcome-time",big);gtk_box_pack_start(GTK_BOX(card),label("Un nouvel espace, à votre image.","welcome-sub"),FALSE,FALSE,0);GtkWidget*sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);gtk_box_pack_start(GTK_BOX(card),sep,FALSE,FALSE,8);gtk_box_pack_start(GTK_BOX(card),label("●  Votre bureau est prêt                         ↗","welcome-foot"),FALSE,FALSE,0);
    GtkWidget*bottom=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,9);gtk_widget_set_size_request(bottom,-1,54);gtk_style_context_add_class(gtk_widget_get_style_context(bottom),"taskbar");gtk_box_pack_start(GTK_BOX(root),bottom,FALSE,FALSE,0);
    start_button=gtk_menu_button_new();gtk_container_add(GTK_CONTAINER(start_button),citron_icon(28));gtk_widget_set_tooltip_text(start_button,"Menu Démarrer");gtk_style_context_add_class(gtk_widget_get_style_context(start_button),"shell-button");gtk_box_pack_start(GTK_BOX(bottom),start_button,FALSE,FALSE,0);
    GtkWidget*pop=gtk_popover_new(start_button);gtk_style_context_add_class(gtk_widget_get_style_context(pop),"popover");GtkWidget*pv=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);gtk_widget_set_margin_start(pv,16);gtk_widget_set_margin_end(pv,16);gtk_widget_set_margin_top(pv,14);gtk_widget_set_margin_bottom(pv,12);gtk_container_add(GTK_CONTAINER(pop),pv);GtkWidget*search=gtk_search_entry_new();gtk_entry_set_placeholder_text(GTK_ENTRY(search),"Rechercher une application…");gtk_box_pack_start(GTK_BOX(pv),search,FALSE,FALSE,0);gtk_box_pack_start(GTK_BOX(pv),label("Applications","tiny"),FALSE,FALSE,0);GtkWidget*grid=gtk_grid_new();gtk_grid_set_row_spacing(GTK_GRID(grid),4);gtk_grid_set_column_spacing(GTK_GRID(grid),5);const char*an[]={"Fichiers","Internet","Notes","Calculatrice","Réglages","Terminal"};const char*ag[]={"▰","◎","▤","＋","⚙","⌘"};for(int i=0;i<6;i++)gtk_grid_attach(GTK_GRID(grid),app_tile(an[i],ag[i]),i%3,i/3,1,1);gtk_box_pack_start(GTK_BOX(pv),grid,FALSE,FALSE,0);gtk_menu_button_set_popover(GTK_MENU_BUTTON(start_button),pop);
    GtkWidget*line=gtk_separator_new(GTK_ORIENTATION_VERTICAL);gtk_box_pack_start(GTK_BOX(bottom),line,FALSE,FALSE,2);
    const char*pin[]={"Fichiers","Internet","Notes","Calculatrice"};const char*pg[]={"▰","◎","▤","＋"};for(int i=0;i<4;i++){GtkWidget*b=gtk_button_new_with_label(pg[i]);gtk_widget_set_tooltip_text(b,pin[i]);gtk_style_context_add_class(gtk_widget_get_style_context(b),"shell-button");g_signal_connect(b,"clicked",G_CALLBACK(pin_clicked),(gpointer)pin[i]);gtk_box_pack_start(GTK_BOX(bottom),b,FALSE,FALSE,0);}
    GtkWidget*fill=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);gtk_box_pack_start(GTK_BOX(bottom),fill,TRUE,TRUE,0);GtkWidget*tray=label("⌃    ◉    ◖","muted");gtk_box_pack_end(GTK_BOX(bottom),tray,FALSE,FALSE,0);task_clock_label=label("--:--","tiny");gtk_box_pack_end(GTK_BOX(bottom),task_clock_label,FALSE,FALSE,5);
    update_clock();g_timeout_add_seconds(1,clock_tick,NULL);
    gtk_widget_show_all(desktop);gtk_main();return 0;
}
