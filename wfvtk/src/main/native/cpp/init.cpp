#include "wfvtk/init.h"
#include <gst/gst.h>

void wfvtk::init() {
    gst_init(nullptr,nullptr);
}