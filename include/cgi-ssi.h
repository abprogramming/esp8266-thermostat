#ifndef __CGISSIDATA_H__
#define __CGISSIDATA_H__


enum {
    TEMP_TGT,
    TEMP_ROOM,
    TEMP_OUTS,
    RELAY_STATE,
    RELAY_FORCEON,
    LOG_TSTAMP,
    SERVER_UPTIME,
    LOG_MIN
};

// Each element in the array corresponds to
// a member of the unnamed enum above
const char *pcConfigSSITags[] =
{
    "tgttemp",
    "roomtemp",
    "outstemp",
    "rstate",
    "forceon",
    "tstamp",
    "uptime",
    "log1","log2","log3","log4","log5",
    "log6","log7","log8","log9","log10",
    "log11","log12","log13","log14","log15",
    "log16","log17","log18","log19","log20",
    "log21","log22","log23","log24","log25",
    "log26","log27","log28","log29","log30",
    "log31","log32","log33","log34","log35",
    "log36","log37","log38","log39","log40",
    "log41","log42","log43","log44","log45",
    "log46","log47","log48","log49","log50",
    "log51","log52","log53","log54","log55",
    "log56","log57","log58","log59","log60",
    "log61","log62","log63","log64","log65",
    "log66","log67","log68","log69","log70",
    "log71","log72","log73","log74","log75",
    "log76","log77","log78","log79","log80",
    "log81","log82","log83","log84","log85",
    "log86","log87","log88","log89","log90",
    "log91","log92","log93","log94","log95",
    "log96","log97","log98","log99","log100"
};

#endif
