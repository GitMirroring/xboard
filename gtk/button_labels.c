#include "gtk/button_labels.h"


#include <libintl.h>
#include <string.h>


char * cancel_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
    static char const gtk_3_c_str[8] = "_Cancel";
# if ENABLE_NLS
    return gettext(gtk_3_c_str);
# else
    return strdup(gtk_3_c_str);
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-cancel");
#else
    #error Not implemented.
#endif
}

char * ok_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
    static char const gtk_3_c_str[4] = "_OK";
# if ENABLE_NLS
    return gettext(gtk_3_c_str);
# else
    return strdup(gtk_3_c_str);
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-ok");
#else
    #error Not implemented.
#endif
}

char * open_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
    static char const gtk_3_c_str[6] = "_Open";
# if ENABLE_NLS
    return gettext(gtk_3_c_str);
# else
    return strdup(gtk_3_c_str);
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-open");
#else
    #error Not implemented.
#endif
}

char * save_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
    static char const gtk_3_c_str[6] = "_Save";
# if ENABLE_NLS
    return gettext(gtk_3_c_str);
# else
    return strdup(gtk_3_c_str);
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-save");
#else
    #error Not implemented.
#endif
}
