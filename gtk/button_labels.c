#include "gtk/button_labels.h"


#include <libintl.h>
#include <string.h>


char * cancel_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_Cancel"));
# else
    return strdup("_Cancel");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-cancel");
#else
    #error Not implemented.
#endif
}

char * ok_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_OK"));
# else
    return strdup("_OK");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-ok");
#else
    #error Not implemented.
#endif
}

char * open_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_Open"));
# else
    return strdup("_Open");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-open");
#else
    #error Not implemented.
#endif
}

char * save_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_Save"));
# else
    return strdup("_Save");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-save");
#else
    #error Not implemented.
#endif
}
