<?php

class Plugin_Macros
{
    const API_URL        = "{API_URL}";       // special url used to get information from provider
    const LIVE_URL       = "{LIVE_URL}";      // live url, used in archive template substitution
    const CH_CATCHUP     = "{CH_CATCHUP}";    // catchup url, used in archive template substitution
    const CGI_BIN        = "{CGI_BIN}";       // Url points to plugin cgi_bin folder
    const SCHEME         = "{SCHEME}";        // stream url scheme (set from playlist)
    const DOMAIN         = "{DOMAIN}";        // stream url domain (set from playlist)
    const PORT           = "{PORT}";          // stream url port (set from playlist)
    const ID             = "{ID}";            // id (set from playlist)
    const PL_DOMAIN      = "{PL_DOMAIN}";     // playlist domain (set from settings or set by provider)
    const SUBDOMAIN      = "{SUBDOMAIN}";     // sub domain (used in edem)
    const OTT_KEY        = "{OTT_KEY}";       // ott key (used in edem)
    const TOKEN          = "{TOKEN}";         // token (set from playlist)
    const COMMAND        = "{COMMAND}";       // command in url
    const S_TOKEN        = "{S_TOKEN}";       // session token (used to get info or playlist from provider)
    const LOGIN          = "{LOGIN}";         // login (set from settings)
    const PASSWORD       = "{PASSWORD}";      // password (set from settings)
    const INT_ID         = "{INT_ID}";        // internal id (reads from playlist)
    const HOST           = "{HOST}";          // host (reads from playlist)
    const SERVER         = "{SERVER}";        // server name (read from settings)
    const SERVER_ID      = "{SERVER_ID}";     // server id (read from settings)
    const DEVICE_ID      = "{DEVICE_ID}";     // device id (read from settings)
    const QUALITY_ID     = "{QUALITY_ID}";    // quality id (set from settings)
    const PROFILE_ID     = "{PROFILE_ID}";    // profile id (read from settings)
    const VAR1           = "{VAR1}";          // Custom capture group variable
    const VAR2           = "{VAR2}";          // Custom capture group variable
    const VAR3           = "{VAR3}";          // Custom capture group variable

    const EPG_DOMAIN     = "{EPG_DOMAIN}";    // epg domain. can be obtain from provider
    const EPG_ID         = "{EPG_ID}";        // epg id (set from playlist)
    const START          = "{START}";         // EPG archive start time (unix timestamp)
    const NOW            = "{NOW}";           // EPG archive current time (unix timestamp)
    const DATE           = "{DATE}";          // EPG date (set by format)
    const TIMESTAMP      = "{TIMESTAMP}";     // EPG time, unix timestamp (set by format)
    const OFFSET         = "{OFFSET}";        // EPG archive current time (unix timestamp)
    const DUNE_IP        = "{DUNE_IP}";       // dune IP address. Useful for using My EPG Server plugin

    const DURATION       = "{DURATION}";      // archive duration (in second) in flussonic archive
    const DURMIN         = "{DURMIN}";        // archive duration (in second) in flussonic archive
    const STOP           = "{STOP}";          // archive end time (unix timestamp)

    const YEAR           = "{YEAR}";          // Year subst template, used in epg_date_format, epg_time_format
    const MONTH          = "{MONTH}";         // Month subst template, used in epg_date_format, epg_time_format
    const DAY            = "{DAY}";           // Day subst template, used in epg_date_format, epg_time_format
    const HOUR           = "{HOUR}";          // Hour subst template, used in epg_time_format
    const MIN            = "{MIN}";           // Minute subst template, used in epg_time_format

    const BUFFERING      = "{BUFFERING}";     // buffering, used in dune_params
}
