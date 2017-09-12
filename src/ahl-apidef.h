
static const char _afb_description_v2_audiohl[] =
    "{\"openapi\":\"3.0.0\",\"$schema\":\"http:iot.bzh/download/openapi/schem"
    "a-3.0/default-schema.json\",\"info\":{\"description\":\"Audio high level"
    " API for AGL applications\",\"title\":\"audiohighlevel\",\"version\":\"1"
    ".0\",\"x-binding-c-generator\":{\"api\":\"audiohl\",\"version\":2,\"pref"
    "ix\":\"audiohlapi_\",\"postfix\":\"\",\"start\":null,\"onevent\":null,\""
    "init\":\"AhlBindingInit\",\"scope\":\"\",\"private\":false}},\"servers\""
    ":[{\"url\":\"ws://{host}:{port}/api/audiohl\",\"description\":\"Audio hi"
    "gh level API for AGL applications.\",\"variables\":{\"host\":{\"default\""
    ":\"localhost\"},\"port\":{\"default\":\"1234\"}},\"x-afb-events\":[{\"$r"
    "ef\":\"#/components/schemas/afb-event\"}]}],\"components\":{\"schemas\":"
    "{\"afb-reply\":{\"$ref\":\"#/components/schemas/afb-reply-v2\"},\"afb-ev"
    "ent\":{\"$ref\":\"#/components/schemas/afb-event-v2\"},\"afb-reply-v2\":"
    "{\"title\":\"Generic response.\",\"type\":\"object\",\"required\":[\"jty"
    "pe\",\"request\"],\"properties\":{\"jtype\":{\"type\":\"string\",\"const"
    "\":\"afb-reply\"},\"request\":{\"type\":\"object\",\"required\":[\"statu"
    "s\"],\"properties\":{\"status\":{\"type\":\"string\"},\"info\":{\"type\""
    ":\"string\"},\"token\":{\"type\":\"string\"},\"uuid\":{\"type\":\"string"
    "\"},\"reqid\":{\"type\":\"string\"}}},\"response\":{\"type\":\"object\"}"
    "}},\"afb-event-v2\":{\"type\":\"object\",\"required\":[\"jtype\",\"event"
    "\"],\"properties\":{\"jtype\":{\"type\":\"string\",\"const\":\"afb-event"
    "\"},\"event\":{\"type\":\"string\"},\"data\":{\"type\":\"object\"}}},\"e"
    "ndpoint_info\":{\"type\":\"object\",\"required\":[\"endpoint_id\",\"type"
    "\",\"device_name\",\"device_uri\"],\"properties\":{\"endpoint_id\":{\"ty"
    "pe\":\"int\"},\"type\":{\"type\":\"enum\"},\"device_name\":{\"type\":\"s"
    "tring\"},\"device_uri_type\":{\"type\":\"string\"}}},\"stream_info\":{\""
    "type\":\"object\",\"required\":[\"stream_id\",\"endpoint_info\"],\"prope"
    "rties\":{\"stream_id\":{\"type\":\"int\"},\"$ref\":\"#/components/schema"
    "s/endpoint_info\"}}},\"x-permissions\":{\"streamcontrol\":{\"permission\""
    ":\"urn:AGL:permission:audio:public:streamcontrol\"},\"routingcontrol\":{"
    "\"permission\":\"urn:AGL:permission:audio:public:routingcontrol\"},\"sou"
    "ndevent\":{\"permission\":\"urn:AGL:permission:audio:public:soundevent\""
    "}},\"responses\":{\"200\":{\"description\":\"A complex object array resp"
    "onse\",\"content\":{\"application/json\":{\"schema\":{\"$ref\":\"#/compo"
    "nents/schemas/afb-reply\"}}}},\"400\":{\"description\":\"Invalid argumen"
    "ts\"}}},\"paths\":{\"/get_sources\":{\"description\":\"Retrieve array of"
    " available audio sources\",\"get\":{\"parameters\":[{\"in\":\"query\",\""
    "name\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\"string\"}"
    "}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"res"
    "ponse\":{\"description\":\"Array of endpoint info structures\",\"type\":"
    "\"array\",\"items\":{\"$ref\":\"#/components/schemas/endpoint_info\"}}},"
    "\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_sinks\":{\"d"
    "escription\":\"Retrieve array of available audio sinks\",\"get\":{\"para"
    "meters\":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\""
    "schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/co"
    "mponents/responses/200\",\"response\":{\"description\":\"Array of endpoi"
    "nt info structures\",\"type\":\"array\",\"items\":{\"$ref\":\"#/componen"
    "ts/schemas/endpoint_info\"}}},\"400\":{\"$ref\":\"#/components/responses"
    "/400\"}}}},\"/stream_open\":{\"description\":\"Request opening a stream\""
    ",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/strea"
    "mcontrol\"},\"parameters\":[{\"in\":\"query\",\"name\":\"audio_role\",\""
    "required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"na"
    "me\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\"}}"
    ",{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":false,\"schema\""
    ":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/re"
    "sponses/200\",\"response\":{\"description\":\"Stream information structu"
    "re\",\"$ref\":\"#/components/schemas/stream_info\"}},\"400\":{\"$ref\":\""
    "#/components/responses/400\"}}}},\"/stream_close\":{\"description\":\"Re"
    "quest closing a stream\",\"get\":{\"x-permissions\":{\"$ref\":\"#/compon"
    "ents/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query\",\""
    "name\":\"stream_id\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\""
    "responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{"
    "\"$ref\":\"#/components/responses/400\"}}}},\"/set_volume\":{\"descripti"
    "on\":\"Set volume\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/"
    "x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query\",\"name\""
    ":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\""
    "in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"schema\":{\""
    "type\":\"int\"}},{\"in\":\"query\",\"name\":\"volume\",\"required\":true"
    ",\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"ramp_tim"
    "e_ms\",\"required\":false,\"schema\":{\"type\":\"int\"}}],\"responses\":"
    "{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#"
    "/components/responses/400\"}}}},\"/get_volume\":{\"description\":\"Get v"
    "olume\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_t"
    "ype\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\""
    ",\"name\":\"endpoint_id\",\"required\":true,\"schema\":{\"type\":\"int\""
    "}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"re"
    "sponse\":{\"description\":\"Endpoint volume value\",\"type\":\"double\"}"
    "},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_property\""
    ":{\"description\":\"Set property value\",\"get\":{\"x-permissions\":{\"$"
    "ref\":\"#/components/x-permissions/streamcontrol\"},\"parameters\":[{\"i"
    "n\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\""
    "type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\""
    ":false,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"prope"
    "rty_name\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\""
    "query\",\"name\":\"value\",\"required\":true,\"schema\":{\"type\":\"stri"
    "ng\"}},{\"in\":\"query\",\"name\":\"ramp_time_ms\",\"required\":false,\""
    "schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/compo"
    "nents/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}"
    "}}},\"/get_property\":{\"description\":\"Get property value\",\"get\":{\""
    "parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":"
    "true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoi"
    "nt_id\",\"required\":false,\"schema\":{\"type\":\"int\"}},{\"in\":\"quer"
    "y\",\"name\":\"property_name\",\"required\":true,\"schema\":{\"type\":\""
    "string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/20"
    "0\",\"response\":{\"description\":\"Property value\",\"type\":\"double\""
    "}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_state\":{"
    "\"description\":\"Set state\",\"get\":{\"x-permissions\":{\"$ref\":\"#/c"
    "omponents/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query"
    "\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"e"
    "num\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"s"
    "chema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"state_name\",\""
    "required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"na"
    "me\":\"state_value\",\"required\":true,\"schema\":{\"type\":\"string\"}}"
    "],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400"
    "\":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_state\":{\"descri"
    "ption\":\"Get state value\",\"get\":{\"parameters\":[{\"in\":\"query\",\""
    "name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\""
    "}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"schema"
    "\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"state_name\",\"requi"
    "red\":true,\"schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\""
    "$ref\":\"#/components/responses/200\",\"response\":{\"description\":\"En"
    "dpoint state value\",\"type\":\"string\"}},\"400\":{\"$ref\":\"#/compone"
    "nts/responses/400\"}}}},\"/post_sound_event\":{\"description\":\"Post so"
    "und event\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permis"
    "sions/soundevent\"},\"parameters\":[{\"in\":\"query\",\"name\":\"event_n"
    "ame\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"quer"
    "y\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\"str"
    "ing\"}},{\"in\":\"query\",\"name\":\"media_name\",\"required\":false,\"s"
    "chema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"audio_contex"
    "t\",\"required\":false,\"schema\":{\"type\":\"object\"}}],\"responses\":"
    "{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#"
    "/components/responses/400\"}}}},\"/subscribe\":{\"description\":\"Subscr"
    "ibe to audio high level events\",\"get\":{\"parameters\":[{\"in\":\"quer"
    "y\",\"name\":\"events\",\"required\":true,\"schema\":{\"type\":\"array\""
    ",\"items\":{\"type\":\"string\"}}}],\"responses\":{\"200\":{\"$ref\":\"#"
    "/components/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/"
    "400\"}}}},\"/unsubscribe\":{\"description\":\"Unubscribe to audio high l"
    "evel events\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"even"
    "ts\",\"required\":true,\"schema\":{\"type\":\"array\",\"items\":{\"type\""
    ":\"string\"}}}],\"responses\":{\"200\":{\"$ref\":\"#/components/response"
    "s/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}}}}"
;

static const struct afb_auth _afb_auths_v2_audiohl[] = {
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:streamcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:soundevent" }
};

 void audiohlapi_get_sources(struct afb_req req);
 void audiohlapi_get_sinks(struct afb_req req);
 void audiohlapi_stream_open(struct afb_req req);
 void audiohlapi_stream_close(struct afb_req req);
 void audiohlapi_set_volume(struct afb_req req);
 void audiohlapi_get_volume(struct afb_req req);
 void audiohlapi_set_property(struct afb_req req);
 void audiohlapi_get_property(struct afb_req req);
 void audiohlapi_set_state(struct afb_req req);
 void audiohlapi_get_state(struct afb_req req);
 void audiohlapi_post_sound_event(struct afb_req req);
 void audiohlapi_subscribe(struct afb_req req);
 void audiohlapi_unsubscribe(struct afb_req req);

static const struct afb_verb_v2 _afb_verbs_v2_audiohl[] = {
    {
        .verb = "get_sources",
        .callback = audiohlapi_get_sources,
        .auth = NULL,
        .info = "Retrieve array of available audio sources",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_sinks",
        .callback = audiohlapi_get_sinks,
        .auth = NULL,
        .info = "Retrieve array of available audio sinks",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "stream_open",
        .callback = audiohlapi_stream_open,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = "Request opening a stream",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "stream_close",
        .callback = audiohlapi_stream_close,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = "Request closing a stream",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_volume",
        .callback = audiohlapi_set_volume,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = "Set volume",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_volume",
        .callback = audiohlapi_get_volume,
        .auth = NULL,
        .info = "Get volume",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_property",
        .callback = audiohlapi_set_property,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = "Set property value",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_property",
        .callback = audiohlapi_get_property,
        .auth = NULL,
        .info = "Get property value",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_state",
        .callback = audiohlapi_set_state,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = "Set state",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_state",
        .callback = audiohlapi_get_state,
        .auth = NULL,
        .info = "Get state value",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "post_sound_event",
        .callback = audiohlapi_post_sound_event,
        .auth = &_afb_auths_v2_audiohl[1],
        .info = "Post sound event",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "subscribe",
        .callback = audiohlapi_subscribe,
        .auth = NULL,
        .info = "Subscribe to audio high level events",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "unsubscribe",
        .callback = audiohlapi_unsubscribe,
        .auth = NULL,
        .info = "Unubscribe to audio high level events",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = NULL,
        .callback = NULL,
        .auth = NULL,
        .info = NULL,
        .session = 0
	}
};

const struct afb_binding_v2 afbBindingV2 = {
    .api = "audiohl",
    .specification = _afb_description_v2_audiohl,
    .info = "Audio high level API for AGL applications",
    .verbs = _afb_verbs_v2_audiohl,
    .preinit = NULL,
    .init = AhlBindingInit,
    .onevent = NULL,
    .noconcurrency = 0
};

