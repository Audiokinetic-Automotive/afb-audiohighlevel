
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
    "\",\"name\"],\"properties\":{\"endpoint_id\":{\"type\":\"int\"},\"type\""
    ":{\"type\":\"enum\"},\"name\":{\"type\":\"string\"}}},\"stream_info\":{\""
    "type\":\"object\",\"required\":[\"stream_id\",\"pcm_name\",\"name\"],\"p"
    "roperties\":{\"stream_id\":{\"type\":\"int\"},\"pcm_name\":{\"type\":\"s"
    "tring\"},\"$ref\":\"#/components/schemas/endpoint_info\"}},\"routing_inf"
    "o\":{\"type\":\"object\",\"required\":[\"routing_id\",\"source_id\",\"si"
    "nk_id\"],\"properties\":{\"routing_id\":{\"type\":\"int\"},\"source_id\""
    ":{\"type\":\"int\"},\"sink_id\":{\"type\":\"int\"}}}},\"x-permissions\":"
    "{\"streamcontrol\":{\"permission\":\"urn:AGL:permission:audio:public:str"
    "eamcontrol\"},\"routingcontrol\":{\"permission\":\"urn:AGL:permission:au"
    "dio:public:routingcontrol\"},\"soundevent\":{\"permission\":\"urn:AGL:pe"
    "rmission:audio:public:soundevent\"}},\"responses\":{\"200\":{\"descripti"
    "on\":\"A complex object array response\",\"content\":{\"application/json"
    "\":{\"schema\":{\"$ref\":\"#/components/schemas/afb-reply\"}}}},\"400\":"
    "{\"description\":\"Invalid arguments\"}}},\"paths\":{\"/get_sources\":{\""
    "description\":\"Retrieve array of available audio sources\",\"get\":{\"p"
    "arameters\":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":fals"
    "e,\"schema\":{\"type\":\"enum\"}}],\"responses\":{\"200\":{\"$ref\":\"#/"
    "components/responses/200\",\"response\":{\"description\":\"Array of endp"
    "oint info structures\",\"type\":\"array\",\"items\":{\"$ref\":\"#/compon"
    "ents/schemas/endpoint_info\"}}},\"400\":{\"$ref\":\"#/components/respons"
    "es/400\"}}}},\"/get_sinks\":{\"description\":\"Retrieve array of availab"
    "le audio sinks\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"a"
    "udio_role\",\"required\":false,\"schema\":{\"type\":\"enum\"}}],\"respon"
    "ses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"response\":{\""
    "description\":\"Array of endpoint info structures\",\"type\":\"array\",\""
    "items\":{\"$ref\":\"#/components/schemas/endpoint_info\"}}},\"400\":{\"$"
    "ref\":\"#/components/responses/400\"}}}},\"/stream_open\":{\"description"
    "\":\"Request opening a stream\",\"get\":{\"x-permissions\":{\"$ref\":\"#"
    "/components/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"que"
    "ry\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\"en"
    "um\"}},{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\""
    "schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\""
    ",\"required\":false,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200"
    "\":{\"$ref\":\"#/components/responses/200\",\"response\":{\"description\""
    ":\"Stream information structure\",\"$ref\":\"#/components/schemas/stream"
    "_info\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/stream"
    "_close\":{\"description\":\"Request closing a stream\",\"get\":{\"x-perm"
    "issions\":{\"$ref\":\"#/components/x-permissions/streamcontrol\"},\"para"
    "meters\":[{\"in\":\"query\",\"name\":\"stream_id\",\"required\":true,\"s"
    "chema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/compon"
    "ents/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}"
    "}},\"/get_available_routings\":{\"description\":\"Retrieve array of avai"
    "lable routing info structures\",\"get\":{\"parameters\":[{\"in\":\"query"
    "\",\"name\":\"audio_role\",\"required\":false,\"schema\":{\"type\":\"enu"
    "m\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\""
    "response\":{\"description\":\"Array of routing info structures\",\"type\""
    ":\"array\",\"items\":{\"description\":\"Routing info structure {routingI"
    "D, sourceID, sinkID }\",\"type\":\"object\"}}},\"400\":{\"$ref\":\"#/com"
    "ponents/responses/400\"}}}},\"/add_routing\":{\"description\":\"Request "
    "a routing connection between available devices\",\"get\":{\"x-permission"
    "s\":{\"$ref\":\"#/components/x-permissions/routingcontrol\"},\"parameter"
    "s\":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"schem"
    "a\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"routing_id\",\"req"
    "uired\":false,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\""
    "$ref\":\"#/components/responses/200\",\"response\":{\"description\":\"Ro"
    "uting information structure\",\"$ref\":\"#/components/schemas/routing_in"
    "fo\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/remove_ro"
    "uting\":{\"description\":\"Request to remove a routing connection betwee"
    "n devices\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permis"
    "sions/routingcontrol\"},\"parameters\":[{\"in\":\"query\",\"name\":\"rou"
    "ting_id\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\""
    ":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\""
    "#/components/responses/400\"}}}},\"/set_endpoint_volume\":{\"description"
    "\":\"Set endpoint volume\",\"get\":{\"x-permissions\":{\"$ref\":\"#/comp"
    "onents/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query\","
    "\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum"
    "\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"sche"
    "ma\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"volume\",\"require"
    "d\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\""
    "ramp_time_ms\",\"required\":false,\"schema\":{\"type\":\"int\"}}],\"resp"
    "onses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$r"
    "ef\":\"#/components/responses/400\"}}}},\"/get_endpoint_volume\":{\"desc"
    "ription\":\"Get endpoint volume\",\"get\":{\"parameters\":[{\"in\":\"que"
    "ry\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\""
    "enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\""
    "schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/compo"
    "nents/responses/200\",\"response\":{\"description\":\"Endpoint volume va"
    "lue\",\"type\":\"double\"}},\"400\":{\"$ref\":\"#/components/responses/4"
    "00\"}}}},\"/set_endpoint_property\":{\"description\":\"Set endpoint prop"
    "erty value\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permi"
    "ssions/streamcontrol\"},\"parameters\":[{\"in\":\"query\",\"name\":\"end"
    "point_type\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\""
    "query\",\"name\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\""
    ":\"int\"}},{\"in\":\"query\",\"name\":\"property_name\",\"required\":tru"
    "e,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"value\""
    ",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\""
    "name\":\"ramp_time_ms\",\"required\":false,\"schema\":{\"type\":\"int\"}"
    "}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"40"
    "0\":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_endpoint_propert"
    "y\":{\"description\":\"Get endpoint property value\",\"get\":{\"paramete"
    "rs\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\"s"
    "chema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\","
    "\"required\":false,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"na"
    "me\":\"property_name\",\"required\":true,\"schema\":{\"type\":\"string\""
    "}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"re"
    "sponse\":{\"description\":\"Property value\",\"type\":\"double\"}},\"400"
    "\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_endpoint_state\":"
    "{\"description\":\"Set endpoint state\",\"get\":{\"x-permissions\":{\"$r"
    "ef\":\"#/components/x-permissions/streamcontrol\"},\"parameters\":[{\"in"
    "\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\""
    "type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\""
    ":true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"state_"
    "name\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"que"
    "ry\",\"name\":\"state_value\",\"required\":true,\"schema\":{\"type\":\"s"
    "tring\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200"
    "\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_endpoint"
    "_state\":{\"description\":\"Get endpoint state value\",\"get\":{\"parame"
    "ters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\""
    "schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\""
    ",\"required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"na"
    "me\":\"state_name\",\"required\":true,\"schema\":{\"type\":\"string\"}}]"
    ",\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"respo"
    "nse\":{\"description\":\"Endpoint state value\",\"type\":\"string\"}},\""
    "400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/post_sound_event\""
    ":{\"description\":\"Post sound event\",\"get\":{\"x-permissions\":{\"$re"
    "f\":\"#/components/x-permissions/soundevent\"},\"parameters\":[{\"in\":\""
    "query\",\"name\":\"event_name\",\"required\":true,\"schema\":{\"type\":\""
    "string\"}},{\"in\":\"query\",\"name\":\"audio_role\",\"required\":false,"
    "\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"media_name\""
    ",\"required\":false,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\","
    "\"name\":\"audio_context\",\"required\":false,\"schema\":{\"type\":\"obj"
    "ect\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\""
    "},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/subscribe\":{\""
    "description\":\"Subscribe to audio high level events\",\"get\":{\"parame"
    "ters\":[{\"in\":\"query\",\"name\":\"events\",\"required\":true,\"schema"
    "\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}}],\"responses\":"
    "{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#"
    "/components/responses/400\"}}}}}}"
;

static const struct afb_auth _afb_auths_v2_audiohl[] = {
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:streamcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:routingcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:soundevent" }
};

 void audiohlapi_get_sources(struct afb_req req);
 void audiohlapi_get_sinks(struct afb_req req);
 void audiohlapi_stream_open(struct afb_req req);
 void audiohlapi_stream_close(struct afb_req req);
 void audiohlapi_get_available_routings(struct afb_req req);
 void audiohlapi_add_routing(struct afb_req req);
 void audiohlapi_remove_routing(struct afb_req req);
 void audiohlapi_set_endpoint_volume(struct afb_req req);
 void audiohlapi_get_endpoint_volume(struct afb_req req);
 void audiohlapi_set_endpoint_property(struct afb_req req);
 void audiohlapi_get_endpoint_property(struct afb_req req);
 void audiohlapi_set_endpoint_state(struct afb_req req);
 void audiohlapi_get_endpoint_state(struct afb_req req);
 void audiohlapi_post_sound_event(struct afb_req req);
 void audiohlapi_subscribe(struct afb_req req);

static const struct afb_verb_v2 _afb_verbs_v2_audiohl[] = {
    {
        .verb = "get_sources",
        .callback = audiohlapi_get_sources,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_sinks",
        .callback = audiohlapi_get_sinks,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "stream_open",
        .callback = audiohlapi_stream_open,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "stream_close",
        .callback = audiohlapi_stream_close,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_available_routings",
        .callback = audiohlapi_get_available_routings,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "add_routing",
        .callback = audiohlapi_add_routing,
        .auth = &_afb_auths_v2_audiohl[1],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "remove_routing",
        .callback = audiohlapi_remove_routing,
        .auth = &_afb_auths_v2_audiohl[1],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_endpoint_volume",
        .callback = audiohlapi_set_endpoint_volume,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_endpoint_volume",
        .callback = audiohlapi_get_endpoint_volume,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_endpoint_property",
        .callback = audiohlapi_set_endpoint_property,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_endpoint_property",
        .callback = audiohlapi_get_endpoint_property,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_endpoint_state",
        .callback = audiohlapi_set_endpoint_state,
        .auth = &_afb_auths_v2_audiohl[0],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_endpoint_state",
        .callback = audiohlapi_get_endpoint_state,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "post_sound_event",
        .callback = audiohlapi_post_sound_event,
        .auth = &_afb_auths_v2_audiohl[2],
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "subscribe",
        .callback = audiohlapi_subscribe,
        .auth = NULL,
        .info = NULL,
        .session = AFB_SESSION_NONE_V2
    },
    { .verb = NULL }
};

const struct afb_binding_v2 afbBindingV2 = {
    .api = "audiohl",
    .specification = _afb_description_v2_audiohl,
    .info = NULL,
    .verbs = _afb_verbs_v2_audiohl,
    .preinit = NULL,
    .init = AhlBindingInit,
    .onevent = NULL,
    .noconcurrency = 0
};

