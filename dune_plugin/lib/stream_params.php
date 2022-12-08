<?php

class Stream_Params
{
    const /* (char *) */ STREAM_TYPE             = 'stream_type';             // type of the stream (hls or mpeg)
    const /* (char *) */ URL_TEMPLATE            = 'uri_template';            // template to create live url stream
    const /* (char *) */ URL_ARC_TEMPLATE        = 'uri_arc_template';        // template to create archive url
    const /* (char *) */ URL_CUSTOM_ARC_TEMPLATE = 'uri_custom_arc_template'; // template to create archive url for custom url
    const /* (char *) */ CU_TYPE                 = 'cu_type';                 // type of archive link (shift or flussonic)
    const /* (char *) */ CU_SUBST                = 'cu_subst';                // substitution word for shift type archive (utc/archive etc.)
    const /* (int)    */ CU_NOW                  = 'cu_now';                  // time now
    const /* (int)    */ CU_START                = 'cu_start';                // epg start time
    const /* (int)    */ CU_OFFSET               = 'cu_offset';               // offset from now to time of the begin archive
    const /* (int)    */ CU_DURATION             = 'cu_duration';             // length of archive part used in flussonic types archive
    const /* (int)    */ CU_STOP                 = 'cu_stop';                 // end of archive part in unix timestamp
    const /* (char*)  */ DUNE_PARAMS             = 'dune_params';             // additional dune params added to the end of the url
}
