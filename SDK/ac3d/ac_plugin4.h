/** ac_plugin.h  -  include file for AC3D plugins www.ac3d.org **/

/** (C) Copyright Inivis Ltd 2003 **/



/** last updated 4th Oct 2003 **/





#ifndef AC_PLUGIN

#define AC_PLUGIN







#ifdef __cplusplus

extern "C"

{

#endif







#ifdef WINDOWS

	#include <windows.h>



	#define message_dialog windows_message_dialog



	#ifdef __cplusplus

		#define AC3D_PLUGIN_FUNC extern "C" __declspec(dllexport)

	#else

		#define AC3D_PLUGIN_FUNC __declspec(dllexport)

	#endif // __cplusplus



#else



	#ifdef __cplusplus

		#define AC3D_PLUGIN_FUNC extern "C"

	#else

             #define AC3D_PLUGIN_FUNC



	#endif // __cplusplus



#endif





/** need booleans to be an integer so that TCL/TK can access them **/

#ifndef Boolean

#define Boolean int

#ifndef FALSE

#define FALSE (0)

#define TRUE (!FALSE)

#endif

#endif



#define M_PI 3.14159265358979323846f





/** structure passed to the plugin init function **/

typedef struct AC3DPluginInitData_t

{

    char *plugin_name; /** name of plugin e.g. testplugin.p **/

    char *plugin_dir_path; /** dir of plugin being called **/

    char *plugin_full_path; /** full file name of plugin being called **/

} AC3DPluginInitData;





/** simple linked list structure - used everywhere in AC3D **/

typedef void *ListData;



typedef struct listitem

{

    ListData data;

    struct listitem *next;

} List;



typedef struct object_t

{

int dummy;

} ACObject; /** private data structure - use functions to access **/



/** object types used in new_object(type) **/

#define OBJECT_NORMAL 0  /** polygon based object **/

#define OBJECT_GROUP 1

#define OBJECT_LIGHT 2

#define OBJECT_WORLD 999  /** toplevel world object type **/





/** windows identifiers **/

#define FRONTWIN (0)

#define SIDEWIN (1)

#define TOPWIN (2)

#define WIN3D (3)



#define list_for_each(list, p)   for (p = list; p != NULL; p = p->next)



typedef struct point3_t

{

    float x, y, z;

} Point3;



typedef struct point2_t

{

    float x, y;

} Point2;





// always use new_vertex*() to create a new vertex

typedef struct vertex_t

{

    float x, y, z;



	// private attributes are here - accessed by fucntions

} Vertex;









/** a surface is a List of SVertex's.  Each v points to a vertex in the objects vertex list, create with new_svertex() **/

typedef struct svertex_t

{

    Vertex *v;

    float tx, ty; /* texture coors */

	Point3 normal; // vertex normal



	// extra private stuff is here

} SVertex;



#define SVERTEX(p) (((Vertex *) ((SVertex *)p)->v)) /** get the vertex out of an SVertex **/







// always use new_surface() to create a new surface

typedef struct surface_t

{

    List *vertlist; /** list of SVertex **/

    unsigned short numvert;

    unsigned short col; // actually the material index

    Point3 normal; // surface normal



	// extra private stuff is in here, use functions to access attributes

} Surface;









/** surface types from surface_get_type() **/

#define SURFACE_POLYGON (0)

#define SURFACE_CLOSEDLINE (1)

#define SURFACE_LINE (2)











typedef struct Col3_t

{

    float r, g, b, a;

} Col3;



typedef struct material_t

{

    Col3 rgb; /* diffuse **/

    Col3 ambient;

    Col3 specular;

    Col3 emissive;

    float shininess;

    float transparency;

    char *name;  /** OK to read name directly but only use material_set_name to change it **/

	void *userdata;

} ACMaterial;



typedef struct ACImageStruct

{

    unsigned short width;

	unsigned short height;

	unsigned short depth;    // e.g. 3 = RGB, 4 = RGBA

    void *data;

    int index;

    char *name;

    int amask;

    char *origname; /** do not set - set automatically in texture_read function **/

	void *userdata;

} ACImage;



/* types of alpha values in picture (used in acimage->amask) */

#define ALPHA_NONE   (0x0000)       /* no alpha info */

#define ALPHA_OPAQUE (0x0001<<0)    /* alpha = 1 */

#define ALPHA_INVIS  (0x0001<<1)    /* alpha = 0 */

#define ALPHA_TRANSP (0x0004<<2)    /* 0 < alpha < 1 */





/** person structure - from get_person()  - use functions to set values whereever possible**/

typedef struct Person_t

{

    Point3 origin; /** x, y, z position **/

    Point3 location; /** x, y, z eye position **/

    Point3 direction; /** rotation of camera in degs **/

    Point3 rotation; /** rotation of objects around origin (degs) **/

    Point3 spin; /** rotation change objects around origin for spinning (degs) **/

    float scale; /** overall scale factor (starts 1.0) **/

    float move_dist; /** move increment **/

    float move_angle; /** move rotate increment **/

    int walk_vehicle; /** actually a Boolean but linked to tcl **/

    Point3 offset;

} Person;







/** some prototypes for AC3D functions **/



#ifdef WINDOWS

#define Prototype  __declspec( dllimport )

#else

#define Prototype

#endif





Prototype double ac_get_version_number();

Prototype char *tcl_eval_file(char *str);

Prototype char *tcl_command(char *fmt, ...);

Prototype void ac_add_command(char *name, void (*function)( ) );

Prototype void ac_add_import_menu_item(char *label, char *command);

Prototype void ac_add_export_menu_item(char *label, char *command);

Prototype void ac_add_tools_menu_item(char *label, char *command, char *desc);

Prototype char *ac_get_load_filename(char *title, char **suffix);

Prototype char *ac_get_import_filename(char *title, char **suffix);

Prototype char *ac_get_save_filename(char *title, char **suffix);

Prototype char *ac_get_export_filename(char *title, char **suffix);

Prototype int ac_get_filesize(char *filename);

Prototype ACObject *ac_get_world();

Prototype Boolean ac_replace_command_function(char *name, void (*function)());

Prototype void ac_add_command_full(char *name, void *function, int numargs, char *args, char *usage, char *desc);

Prototype char *ac_get_export_folder(char *title);





// general util

Prototype char *string_append_string(char **str, char *add); // e.g. char *s = NULL; string_append_string(&s, "hello"); call myfree on string when you have finished with it

Prototype char *ac_filename_concat_path_file(char *path, char *file, char *outret);







/** LINKED LIST STUFF **/

Prototype List *list_add_item(List **list, void *data);

Prototype List *list_add_item_unique(List **list, void *data);

Prototype List *list_add_item_head(List **list, void *data);

Prototype int list_remove_item(List **list, void *data);

Prototype void list_print(List *l);

Prototype int list_count(List *l);

Prototype List *list_append_list(List **list, List *part);

Prototype List *list_append_list_unique(List **list, List *part);

Prototype int list_index(List *l, void *data);

Prototype Boolean list_replace_item(List *l, void *search, void *replace);

Prototype Boolean list_replace_item_all(List *l, void *search, void *replace);

Prototype void *list_get_item(List *l, int pos);

Prototype void list_free(List **l);

Prototype void list_free_all(List **l);

Prototype List *list_clone(List *l);

Prototype List *list_reverse(List **l);

Prototype List *list_end(List *l);

Prototype void *list_last_item(List *l);

Prototype void list_sort(List *list, int (*compare)(void *, void *));

Prototype List *list_clone_all(List *l, int item_size);

Prototype Boolean list_insert_at(List **l, int pos, void *item);

Prototype Boolean list_insert_item(List **l, void *before, void *item);

Prototype ListData *list_create_array(List *l, int numitems); // free with myfree()















/** SURFACE FILTERS  - NO LONGER SUPPORTED do not use **/



typedef void (*ac_surface_filter_func)( ACObject *object, Surface *surface);



typedef struct ACSurfaceFilter_t

{

    char *name; /** name for the menu e.g. "Bevel..." **/

    char *desc; /** description of the function e.g. for balloon help **/

    ac_surface_filter_func func; /** ptr to the function that's called with each surface **/

    int flags;

    char *internal_name; /** auto generated name used in the TCL command to refer to this filter **/

} ACSurfaceFilter;



#define AC_SFILTER_FLAG_NO_UNDO (1<<0)  /** don't setup an undo **/

#define AC_SFILTER_FLAG_NO_MENU_ITEM (1<<1)  /** don't add a menu item **/



Prototype ACSurfaceFilter *ac_register_surface_filter(char *name, char *desc, ac_surface_filter_func func, int flags);







/** VERTICES **/

Prototype Vertex *new_vertex(void);

Prototype Vertex *new_vertex_set(float x, float y, float z);

Prototype Vertex *vertex_clone(Vertex *p);

Prototype void vertex_set_point(Vertex *v, Point3 *p);







/** OBJECTS **/

Prototype void ac_object_get_contents(ACObject *ob, int *numvert, int *numsurf, int *numchildren, List **points, List **surfaces, List **children);

Prototype ACObject *new_object(int type);

Prototype Vertex *ac_object_get_vertex_at_index(ACObject *ob, int idx);

Prototype int ac_object_get_vertex_index(ACObject *ob, Vertex *v);

Prototype void object_free(ACObject *ob);

Prototype void object_delete(ACObject *ob); /** removes from parent and cleansup **/

Prototype void object_set_name(ACObject *ob, char *name);

Prototype void object_set_url(ACObject *ob, char *url);

Prototype void object_set_userdata(ACObject *ob, char *dat); /** set the objectdata string **/

Prototype char *object_get_userdata(ACObject *ob); /** get the objectdata string **/

Prototype void object_add_vertex(ACObject *ob, Vertex *p);

Prototype Boolean object_delete_vertex(ACObject *ob, Vertex *p);

Prototype Boolean object_remove_vertex(ACObject *ob, Vertex *v);

Prototype Vertex *object_add_new_vertex(ACObject *ob, Point3 *p);

Prototype void object_add_vertices(ACObject *ob, List *l);

Prototype void object_add_surface(ACObject *ob, Surface *p);

Prototype void object_add_surface_head(ACObject *ob, Surface *p);

Prototype void object_add_surfaces(ACObject *ob, List *l);

Prototype void object_add_child(ACObject *ob, ACObject *kid);

Prototype void object_add_children(ACObject *ob, List *kids);

Prototype void object_remove_child(ACObject *ob, ACObject *kid); /** automatically deletes obeject if it ends up empty **/

Prototype void object_remove_child_nocleanup(ACObject *ob, ACObject *kid);

Prototype Boolean object_reparent(ACObject *ob, ACObject *newparent);

Prototype ACObject *object_parent(ACObject *ob); /** get the object's parent **/

Prototype void object_delete(ACObject *ob);  /** garbage collects **/

Prototype void object_delete_nocleanup(ACObject *ob);

Prototype void object_free(ACObject *ob);

Prototype ACObject *object_of_vertex(Vertex *pt); /** find object that has this vertex - pass world as ob to search all **/

Prototype ACObject *object_of_surface(Surface *s); /** find object that has this surface **/

Prototype List *surfaces_of_object_vertex(ACObject *ob, Vertex *pt); /** remember to free the list returned **/

Prototype int object_set_surface_type(ACObject *ob, int type); /** returns number of surfaces changed **/

Prototype ACObject *object_top_ancestor(ACObject *ob); /** find top ancesstor which isn't the world object **/

Prototype void object_ungroup(ACObject *ob);

Prototype ACObject *object_clone(ACObject *ob);

Prototype int object_set_col_index(ACObject *ob, int indexc);

Prototype void object_reverse_normals(ACObject *ob);

Prototype void object_delete_surface(ACObject *ob, Surface *s);

Prototype int object_set_surface_shading(ACObject *ob, Boolean g);

Prototype int object_set_twosided_faces(ACObject *ob, Boolean what);

Prototype int object_point_index(ACObject *ob, Point3 *p);

Prototype Vertex *object_find_vertex_point(ACObject *ob, Point3 *p);

Prototype Vertex *object_add_new_vertex_reuse(ACObject *ob, Point3 *p);

Prototype void object_replace_surfaceref(ACObject *ob, Vertex *f, Vertex *r);

Prototype int object_subdivide_surfaces(ACObject *ob, float spike, List **newsurfaces);

Prototype Boolean object_exists(ACObject *ob);

Prototype void object_fragment(ACObject *ob);

Prototype void object_set_texture_repeat(ACObject *ob, float tx, float ty);

Prototype void object_set_texture_offset(ACObject *ob, float tx, float ty);

Prototype void object_set_visible(ACObject *ob, Boolean vis);

Prototype Boolean object_is_selected(ACObject *ob);

Prototype void calc_texture_coors(ACObject *ob, int win);

Prototype void translate_object(ACObject *ob, Point3 *diff);

Prototype void translate_object_abs(ACObject *ob, Point3 *pos);

Prototype void translate_objectlist(List *oblist, Point3 *diff);

Prototype void set_global_matrix_rotate(float rx, float ry, float rz); /** angles in radians **/

Prototype void rotate_object(ACObject *ob); /** rotate with matrix set by set_global_matrix_rotate() **/

Prototype int ac_object_add_vertices_from_float_array(ACObject *ob, Point3 *pts, int num);

Prototype void object_scale(ACObject *ob, float x, float y, float z);

Prototype void translate_object(ACObject *ob, Point3 *diff);

Prototype void translate_object_zero(ACObject *ob);

Prototype void translate_object_abs(ACObject *ob, Point3 *pos);





Prototype List *ac_object_get_childrenlist(ACObject *ob);

Prototype List *ac_object_get_surfacelist(ACObject *ob);

Prototype List *ac_object_get_vertexlist(ACObject *ob);

Prototype int ac_object_get_num_vertices(ACObject *ob);

Prototype int ac_object_get_num_surfaces(ACObject *ob);

Prototype int ac_object_get_num_children(ACObject *ob);

Prototype int ac_object_get_type(ACObject *ob);

Prototype ACObject *ac_object_get_parent(ACObject *ob);

Prototype int ac_object_get_texture_index(ACObject *ob);

Prototype float ac_object_get_texture_repeat_x(ACObject *ob);

Prototype float ac_object_get_texture_repeat_y(ACObject *ob);

Prototype float ac_object_get_texture_offset_x(ACObject *ob);

Prototype float ac_object_get_texture_offset_y(ACObject *ob);

Prototype Point3 *ac_object_get_centre(ACObject *ob);

Prototype char *ac_object_get_data(ACObject *ob);

Prototype char *ac_object_get_url(ACObject *ob);

Prototype char *ac_object_get_name(ACObject *ob);

Prototype Boolean ac_object_has_texture(ACObject *ob);

Prototype void ac_object_set_centre(ACObject *ob, Point3 *t);

Prototype Vertex *object_get_vertex_at(ACObject *ob, int i);

Prototype List *ac_object_get_triangle_surfaces(ACObject *ob);  // free with ac_surfacelist_free(&result)

Prototype int ac_object_remove_vertexlist(ACObject *ob, List *vlist);





Prototype ACObject *find_named_object( char *name, ACObject *startat);

Prototype void object_add_vertex_head(ACObject *ob, Vertex *p);



Prototype void object_validate(ACObject *top, int level);  // check consistency for debugging purposes only





/** these add functions are badly named :- they only create stuff, you need to

    call object_add_child(parent, ACObject); to actually add the returned value **/

Prototype ACObject *add_rect(int win, Point3 *start, Point3 *end);

Prototype ACObject *add_grid(int win, Point3 *start, Point3 *end);

Prototype ACObject *add_mesh(int win, Point3 *start, Point3 *end);

Prototype ACObject *new_arc(int win, float radius ,float startangle, float endangle, int sides);

Prototype ACObject *add_circle(int win, Point3 *start, Point3 *end);

Prototype ACObject *add_disk(int win, Point3 *start, Point3 *end);

Prototype ACObject *add_cube(int win, Point3 *start, Point3 *end);

Prototype ACObject *add_sphere(int win, Point3 *start, Point3 *end);

Prototype ACObject *create_regular_cube(Point3 *cent);





/** LIGHTS **/

Prototype List *ac_get_lights_list();

Prototype Boolean object_is_light(ACObject *ob);











/** SURFACEs **/

Prototype Surface *new_surface(void);

Prototype void surface_free(Surface *s);

Prototype SVertex *surface_add_vertex(Surface *s, Vertex *p, float tx, float ty);

Prototype SVertex *surface_add_vertex_head(Surface *s, Vertex *p, float tx, float ty);

Prototype void surface_set_shading(Surface *s, Boolean g);

Prototype int surface_replace_vertex_all(Surface *s, Vertex *search, Vertex *replace);

Prototype Boolean surface_delete_svertex(Surface *s, SVertex *sv); /** remove vertex references from a surface - doesnt free vertex **/

Prototype int surface_remove_vertex(Surface *s, Vertex *v); /** remove vertex references from a surface - doesnt free vertex **/

Prototype void surface_copy_properties(Surface *from, Surface *to);

Prototype SVertex *new_svertex(Vertex *vert, float tx, float ty);

Prototype void svertex_free(SVertex *sv);

Prototype Boolean surface_contains_vertex(Surface *s, Vertex *v);

Prototype SVertex *surface_get_svertex(Surface *s, Vertex *v);

Prototype void surface_set_type(Surface *s, int type);

Prototype Surface *surface_clone(Surface *s, ACObject *newob);  /** reuses existing vertices **/

Prototype void surface_add_quad_refs(Surface *s, Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4);

Prototype void surface_add_tri_refs(Surface *s, Vertex *v1, Vertex *v2, Vertex *v3);

Prototype Boolean surface_subdivide(ACObject *ob, Surface *s, float spike_factor);

Prototype void surface_set_twosided(Surface *s, Boolean two);

Prototype Surface *surface_make_hole(ACObject *ob, Surface *s, float hole_percent, Boolean hole);

Prototype Vertex *surface_insert_vertex(Surface *s, Vertex *v1, Vertex *v2, Boolean *newvertexwascreted);

Prototype List *surfaces_of_vertex(Vertex *v);

Prototype void surface_change_vertex_order(Surface *s);

Prototype void surfaces_calc_texture_coors(List *surfacelist, int win);

Prototype void ac_surface_set_normal(Surface *s, Point3 *normal);

Prototype List *surface_get_triangulations(Surface *s);

Prototype SVertex *surface_get_svertex_at(Surface *s, int indexnum);

Prototype void ac_surfacelist_free(List **slist);

Prototype List *ac_surfacelist_get_objects(List *slist);  // call list_free on result

Prototype List *ac_surfacelist_get_vertices(List *slist);  // call list_free on result

Prototype int surface_get_shading(Surface *s); // returns boolean shaded/unshaded

Prototype int surface_get_twosided(Surface *s);  // returns boolean

Prototype int surface_get_type(Surface *s); // returns SURFACE_*

Prototype SVertex *svertex_clone(SVertex *sv);

Prototype Point3 *ac_surface_get_normal(Surface *s);







// EDGES



typedef struct ACEdge_t

{

	Vertex *v1, *v2;

	int connected; // number of surfaces using this edge

	List *surfacelist; // the surfaces

	void *userdata; // anything you like

} ACEdge;





Prototype ACEdge *new_edge(Surface *s, Vertex *v1, Vertex *v2);

Prototype void ac_edge_free(ACEdge *e);

Prototype void ac_edgelist_free(List **e);

Prototype List *ac_object_get_edges(ACObject *ob); // free the returned edgelist

Prototype List *ac_selection_get_edges(Boolean wholesurfaces);

Prototype List *ac_edgelist_remove_internal_edges(List **e);  // keeps only the edges that reference one surface

Prototype List *ac_selection_get_edges_external();

Prototype void ac_edgelist_print(List *e);

Prototype List *ac_edgelist_get_surfaces(List *e);

Prototype Vertex *ac_edge_insert_vertex(ACEdge *e, Boolean *vertexwasnew); // returns list of new allocated vertices (which may be less than edges that were split)

Prototype List *ac_edgelist_get_by_surface(List *e, Surface *s); // free the returned list but not the edges

Prototype ACEdge *ac_edgelist_get_by_vertices(List *e, Vertex *v1, Vertex *v2); // free the returned list but not the edges

Prototype List *ac_edgelist_get_connected(List *edgelist, ACEdge *edge, Vertex *firstv);

Prototype List *ac_edgelist_get_connected_surfaces(List *e, Surface *s);

Prototype void ac_edge_add_surface(ACEdge *e, Surface *s);













/** SELECTION **/



/* selectmodes */

#define SELECT_TREE 0 /** the group button used to be 'tree' **/

#define SELECT_GROUP 0

#define SELECT_OBJECT 1

#define SELECT_SURFACE 2

#define SELECT_VERTEX 3



Prototype void clear_selection();

Prototype int ac_get_current_selection(List **vertexlist, List **surfacelist, List **objectlist); // do not use, use ac_selected_get_* instead

Prototype void select_all(void);

Prototype void select_by_name(char *name);

Prototype void ac_selection_select_objectlist(List *obs);

Prototype void unselect_by_name(char *name);

Prototype Boolean nothing_is_selected(void);

Prototype List *find_selected_whole_surfaces(void); /** free the list when you've finished with it **/

Prototype void fit_selected_all();



Prototype List *ac_selection_get_vertices(); // call in vertex select mode, free list after

Prototype List *ac_selection_get_surfaces(); // call in surface select mode, free list after

Prototype List *ac_selection_get_objects(); // call in object or group select mode, free list after

Prototype List *ac_selection_get_groups(); // gets all selected toplevel obs, call in object or group select mode, free returned list

Prototype List *ac_selection_get_whole_surfaces_all(void); // any select mode, free the list when you've finished with it

Prototype List *ac_selection_get_vertices_all(); // in vertex/surface modes et all selected vertices

Prototype List *ac_selection_get_poly_objects(); // call in object or group select mode, free returned list

Prototype void ac_select_object(ACObject *ob); // can be called in any select mode

Prototype void ac_unselect_object(ACObject *ob); // can be called in any select mode

Prototype void ac_selection_select_objectlist(List *obs); // replaces select_objectlist



Prototype void ac_selection_select_vertexlist(List *l);

Prototype void ac_selection_select_surfacelist(List *l);

Prototype void ac_selection_select_by_surfaces(List *surfacelist);

Prototype void ac_selection_select_by_surface(Surface *s);

Prototype void ac_selection_deselect_vertexlist(List *l);

Prototype ACObject *get_current_object(); // NULL if no object or > 1 selected



typedef void *ACSelection; // private data

Prototype ACSelection *ac_selection_get_snapshot();

Prototype void ac_selection_restore_from_snapshot(ACSelection *replacement);

Prototype void ac_selection_free(ACSelection *sel);





/** NORMALS **/

Prototype void selected_calc_normals_force();

Prototype void object_calc_normals_force(ACObject *ob);

Prototype void selected_calc_normals(); /** won't do anything if the recal-normals setting is off **/

Prototype void object_calc_normals(ACObject *ob);

Prototype void objectlist_calc_normals(List *oblist);









/** MATERIAL AND COLOUR **/

Prototype void surface_set_rgb_long(Surface *s, long rgb); /** allocates a new material and uses that if needed **/

Prototype void surface_set_col(Surface *s, int matindex);

Prototype void long_rgb_to_floats(long rgb, float *r, float *g, float *b);

Prototype void long_rgb_to_floatv(long rgb, float *f);

Prototype long rgb_floats_to_long(float r, float g, float b);

Prototype long rgb_floatv_to_long(float *f);

Prototype long ac_material_index_to_rgb(long indexc);

Prototype void index_to_3f(long indexc, float *r, float *g, float *b);

Prototype ACMaterial *ac_palette_get_material(int num);

Prototype long rgb_to_index(long rgbcol); /** makes a new palette entry if the rgb values is not already used **/

Prototype int ac_get_num_palette(); /** total number of defined palette entries **/

Prototype ACMaterial *new_material();

Prototype ACMaterial *new_material_rgb(long rgb);

Prototype long ac_palette_append_material(ACMaterial *m); /** append this material onto the palette list i.e. doesn't just copy contents **/

Prototype ACMaterial *new_material_copy(ACMaterial *cp);

Prototype void ac_material_set_name(ACMaterial *m, char *name);

Prototype int ac_palette_get_new_material_index(ACMaterial *m); /** if this material exists then return it's index otherwise, allocate a new one, COPY the contents from m and return it's index **/

Prototype int object_surfaces_find_same_material(ACObject *ob);  // returns index of material used in object, -1 if multiple materials are used









/** TEXTURE HANDLING and IMAGE LOADERS FOR TEXTURES/BACKGROUND IMAGES **/

typedef ACImage *(*ac_texture_load_func)(char *filename);



Prototype int ac_register_texture_loader(char *suffix, char *desc, ac_texture_load_func func);

Prototype ACImage *new_acimage(char *name);

Prototype void ac_image_set_dim(ACImage *i, int width, int height, int depth);

Prototype void ac_image_set_data(ACImage *i, unsigned char *data);

Prototype void free_acimage(ACImage *i);

Prototype char *texture_id_to_name(int id);

Prototype ACImage *texture_id_to_image(int id);

Prototype Boolean search_and_set_texture(ACObject *ob, char *geometryfilename, char *texturename);

Prototype void object_texture_set(ACObject *ob, long id);

Prototype int add_new_texture_opt(char *name, char *origname); /** doesn't reload textures with the same name **/

Prototype int add_new_texture_reload(char *name, char *origname); /** force the texture to be loaded, even if it has been loaded before **/

Prototype char *search_texture(char *geometryfilename, char *texturename); /** find the ful path for the texture (searches geomfile dir and other places) **/

Prototype int ac_get_num_textures();

Prototype ACImage *texture_id_to_image(int id);





// FILE Importers and Exporters



typedef Boolean (*ac_file_settings_func)(); // function to be called before import/export, to alter settings, returning FALSE will cancel file op.



// IMPORTERS

typedef ACObject *(*ac_file_importer_func)(char *filename);

Prototype int ac_register_file_importer(char *name, char *suffix, char *desc, ac_file_importer_func func, char *info);

Prototype Boolean ac_replace_importer_function(char *suffix, ac_file_importer_func func); /** suffix e.g ".wrl" **/

Prototype Boolean ac_importer_set_settings_function(char *suffix, ac_file_settings_func func); // to be called before import



// EXPORTERS

typedef Boolean (*ac_file_exporter_func)(char *filename, ACObject *object);

Prototype int ac_register_file_exporter(char *name, char *suffix, char *desc, ac_file_exporter_func func, char *info);

Prototype Boolean ac_replace_exporter_function(char *suffix, ac_file_exporter_func func);

Prototype Boolean ac_exporter_set_settings_function(char *suffix, ac_file_settings_func func); // to be called before export









/** Person **/

Prototype Person *get_person(); /** returns ptr to Person structure **/

Prototype Boolean ac_person_is_walk_vehicle();

Prototype void person_set_walk();

Prototype void person_set_spin();

Prototype void ac_person_scale_mult(float zoommult);

Prototype void ac_person_scale_div(float zoomdiv);

Prototype void ac_person_set_rotation(Point3 *rotdegs);

Prototype void ac_person_add_rotation(Point3 *rotdegs);

Prototype void person_reset();

Prototype void person_turn_left(float angle); /** angle in degrees **/

Prototype void person_turn_right(float angle);

Prototype void person_turn_down(float angle);

Prototype void person_turn_up(float angle);

Prototype void person_move_left(float dist);

Prototype void person_move_right(float dist);

Prototype void person_move_forward(float dist);

Prototype void person_move_back(float dist);

Prototype void person_move_up(float dist);

Prototype void person_move_down(float dist);

Prototype void person_set_location(Point3 *p);

Prototype void person_set_offset(Point3 *offset);  /** for spin mode **/

Prototype void person_zero_offset(); /** spin mode origin **/

Prototype void person_get_spin_rotation(float *x, float *y);

Prototype void person_set_spin_rotation(float x, float y);

Prototype void person_add_spin_rotation(float x, float y);







/** used to get the size of a window in win_get_geom() : **/

typedef struct wingeom_t

{

    int x, y, width, height;

} wingeom;





/** MISC **/



Prototype int message_dialog(char *fmt, ...);

Prototype int windows_message_dialog(char *fmt, ...); // don't use this - use message_dialog

Prototype void *myalloc(int size);

Prototype void myfree(void *addr);

Prototype void win_get_geom(int win, wingeom *wg);

Prototype unsigned char *ac_win_get_pixels(int win, int *width, int *height);

Prototype int ac_get_tokens(char *s, int *argc, char *argv[], int maxargs);

Prototype void debugf(char *fmt, ...); // outputs when program is run with '-debug'

Prototype void redraw_all();

Prototype void set_progress_bar(int percent);

Prototype void set_progress_bar_tick(int max, int sofar); // calculates percentage given max and sofar, only displays changes on 10% boundaries

Prototype void ac_unbackslash_string(char *str);

Prototype void display_message(char *fmt, ...); // display a string on the bottom of the AC3D window

Prototype void display_status();  // update the UI

Prototype int ac_object_vertexlist_set_indexes(ACObject *ob); // note that the vertex indexes are set frequestly in AC3D should be considered as non-permanent

Prototype int ac_vertex_get_index(Vertex *v); // get the position in the list as set by ac_object_vertexlist_set_indexes

Prototype void ac_add_import_menu_item(char *label, char *command);

Prototype void ac_add_export_menu_item(char *label, char *command);

Prototype void ac_add_tools_menu_item(char *label, char *command, char *desc);

Prototype void ac_add_surface_menu_item(char *label, char *command, char *desc);

Prototype void ac_add_vertex_menu_item(char *label, char *command, char *desc);

Prototype void ac_add_object_menu_item(char *label, char *command, char *desc);





/** access to PREFS **/



#define PREF_INT 1

#define PREF_DOUBLE 2

#define PREF_HEX 4

#define PREF_STRING 8

#define PREF_KEY 16

#define PREF_BOOLEAN 32

#define PREF_NOSAVE 128 // don't save this value in the AC3D prefs file



typedef struct prefspec

{

    char *name;

    int type; // PREF_

    void *addr; // pointer to current value

} PrefSpec;



// Note that string values must be allocated with  STRING();



Prototype PrefSpec *ac_prefs_get_spec(char *name);  /** returns NULL if not found **/





/** best to use these functions to access prefs: **/



Prototype int ac_prefs_list();  /** list prefs names with types  - returns total number of prefs **/

Prototype int ac_prefs_get_int(char *name);

Prototype Boolean ac_prefs_get_boolean(char *name);

Prototype double ac_prefs_get_double(char *name);

Prototype int ac_prefs_get_hex(char *name);

Prototype char *ac_prefs_get_string(char *name);

Prototype char *ac_prefs_get_key(char *name);

Prototype void ac_tcl_link_pref(PrefSpec *p); // make this preference available in tcl as pref_<p->name>

Prototype void prefs_append_prefspec(PrefSpec *n); // add a new pref to the main list







// undo

Prototype void add_undoable_all(char *name);

Prototype void add_undoable_vertex_positions(char *name, List *vertexlist);

Prototype void add_undo_texture_coords(char *name, List *sl, List *svl);

Prototype void add_undoable_world_add_objectlist(char *name, List *obs);

Prototype void add_undoable_world_add_object(char *name, ACObject *ob);

Prototype void add_undoable_object_centres(char *name, List *oblist);

















#define MIN(a,b) ((a<b)?a:b)

#define MAX(a,b) ((a>b)?a:b)



#define SUBTRACTPOINTS(b,a) {(a)->x-=(b)->x ; (a)->y-=(b)->y ; (a)->z-=(b)->z; }



#define ADDPOINTS(b,a) {(a)->x+=(b)->x ; (a)->y+=(b)->y ; (a)->z+=(b)->z; }

#define SAMEPOINT(a, b) ( ((a)->x == (b)->x) && ((a)->y == (b)->y) && ((a)->z ==(b)->z) )

#define MULTIPLYPOINTS(b,a) {(a)->x*=(b)->x ; (a)->y*=(b)->y ; (a)->z*=(b)->z; }

#define ABSPOINT(a) {(a)->x = fabs( (a)->x); (a)->y = fabs( (a)->y); (a)->z = fabs( (a)->z);  }



#define DIVIDEPOINT(a, d) {(a)->x/=d ; (a)->y/=d ; (a)->z/=d; }

#define MULTIPLYPOINT(a, f) {(a)->x*=f ; (a)->y*=f ; (a)->z*=f; }

#define AVERAGEPOINTS(a, b) {(b)->x = ((a)->x + (b)->x)/2.0; (b)->y = ((a)->y + (b)->y)/2.0; (b)->z = ((a)->z + (b)->z)/2.0;}





#define MIDPOINT(a, b, c) {(c)->x = ((a)->x + (b)->x)/2.0; (c)->y = ((a)->y + (b)->y)/2.0; (c)->z = ((a)->z + (b)->z)/2.0;}





#define NEGATEPOINT(a) {(a)->x=-(a)->x; (a)->y=-(a)->y ; (a)->z=-(a)->z; }

#define INVERTPOINT(p) { (p)->x = -(p)->x; (p)->y = -(p)->y; (p)->z = -(p)->z; }

#define ZEROPOINT(p) { (p)->x = 0; (p)->y = 0; (p)->z = 0; }



#define DOTPRODUCT(p1, p2)  ((float)((p1)->x*(p2)->x + (p1)->y*(p2)->y + (p1)->z*(p2)->z))

#define CROSSPRODUCT(p1, p2, r)  { (r)->x = ((p1)->y * (p2)->z - (p1)->z * (p2)->y); (r)->y = -((p1)->x * (p2)->z - (p1)->z * (p2)->x);  (r)->z = ((p1)->x * (p2)->y - (p1)->y * (p2)->x); }



#define POINTISZERO(p) ( ((p)->x == 0) && ((p)->y == 0) && ((p)->z == 0) )

#define POINTEQ(p, a, b, c) ( ((p)->x == (a)) && ((p)->y == (b)) && ((p)->z == (c)) )

#define SETPOINT(p,a,b,c) {(p)->x=(float)(a); (p)->y=(float)(b); (p)->z=(float)(c);}

#define POINTCOPY(b, a) {(a)->x = (b)->x; (a)->y = (b)->y; (a)->z = (b)->z;}

#define sqr(x) ((x)*(x))



#define DEGTORAD(f) (((f)*M_PI)/180)

#define RADTODEG(f) (((f)*180.0)/M_PI)



#define STRINGISEMPTY(s) ( (s == NULL) || (*(char *)s == 0) )

#define STRING(s)  (strcpy((char *)myalloc(strlen(s)+1), s))











#ifdef __cplusplus

}

#endif





#endif /** AC_PLUGIN **/





