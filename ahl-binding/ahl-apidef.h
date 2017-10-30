
static const char _afb_description_v2_ahl_4a[] =
    "{\"openapi\":\"3.0.0\",\"info\":{\"description\":\"Audio high level API "
    "for AGL applications\",\"title\":\"audiohighlevel\",\"version\":\"1.0\","
    "\"x-binding-c-generator\":{\"api\":\"ahl-4a\",\"version\":2,\"prefix\":\""
    "audiohlapi_\",\"postfix\":\"\",\"start\":null,\"onevent\":\"AhlOnEvent\""
    ",\"init\":\"AhlBindingInit\",\"scope\":\"\",\"private\":false}},\"server"
    "s\":[{\"url\":\"ws://{host}:{port}/api/audiohl\",\"description\":\"Audio"
    " high level API for AGL applications.\",\"variables\":{\"host\":{\"defau"
    "lt\":\"localhost\"},\"port\":{\"default\":\"1234\"}},\"x-afb-events\":[{"
    "\"$ref\":\"#/components/schemas/afb-event\"}]}],\"components\":{\"schema"
    "s\":{\"afb-reply\":{\"$ref\":\"#/components/schemas/afb-reply-v2\"},\"af"
    "b-event\":{\"$ref\":\"#/components/schemas/afb-event-v2\"},\"afb-reply-v"
    "2\":{\"title\":\"Generic response.\",\"type\":\"object\",\"required\":[\""
    "jtype\",\"request\"],\"properties\":{\"jtype\":{\"type\":\"string\",\"co"
    "nst\":\"afb-reply\"},\"request\":{\"type\":\"object\",\"required\":[\"st"
    "atus\"],\"properties\":{\"status\":{\"type\":\"string\"},\"info\":{\"typ"
    "e\":\"string\"},\"token\":{\"type\":\"string\"},\"uuid\":{\"type\":\"str"
    "ing\"},\"reqid\":{\"type\":\"string\"}}},\"response\":{\"type\":\"object"
    "\"}}},\"afb-event-v2\":{\"type\":\"object\",\"required\":[\"jtype\",\"ev"
    "ent\"],\"properties\":{\"jtype\":{\"type\":\"string\",\"const\":\"afb-ev"
    "ent\"},\"event\":{\"type\":\"string\"},\"data\":{\"type\":\"object\"}}},"
    "\"endpoint_info\":{\"type\":\"object\",\"required\":[\"endpoint_id\",\"t"
    "ype\",\"device_name\",\"device_uri\"],\"properties\":{\"endpoint_id\":{\""
    "type\":\"int\"},\"type\":{\"type\":\"enum\"},\"device_name\":{\"type\":\""
    "string\"},\"device_uri_type\":{\"type\":\"string\"}}},\"stream_info\":{\""
    "type\":\"object\",\"required\":[\"stream_id\",\"state\",\"mute\",\"endpo"
    "int_info\"],\"properties\":{\"stream_id\":{\"type\":\"int\"},\"state\":{"
    "\"type\":\"int\"},\"mute\":{\"type\":\"int\"},\"device_uri\":{\"type\":\""
    "string\"},\"$ref\":\"#/components/schemas/endpoint_info\"}}},\"x-permiss"
    "ions\":{\"streamcontrol\":{\"permission\":\"urn:AGL:permission:audio:pub"
    "lic:streamcontrol\"},\"endpointcontrol\":{\"permission\":\"urn:AGL:permi"
    "ssion:audio:public:endpointcontrol\"},\"audiostream\":{\"permission\":\""
    "urn:AGL:permission:audio:public:audiostream\"},\"soundevent\":{\"permiss"
    "ion\":\"urn:AGL:permission:audio:public:soundevent\"}},\"responses\":{\""
    "200\":{\"description\":\"A complex object array response\",\"content\":{"
    "\"application/json\":{\"schema\":{\"$ref\":\"#/components/schemas/afb-re"
    "ply\"}}}},\"400\":{\"description\":\"Invalid arguments\"}}},\"paths\":{\""
    "/get_sources\":{\"description\":\"Retrieve array of available audio sour"
    "ces\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"audio_role\""
    ",\"required\":true,\"schema\":{\"type\":\"string\"}}],\"responses\":{\"2"
    "00\":{\"$ref\":\"#/components/responses/200\",\"response\":{\"descriptio"
    "n\":\"Array of endpoint info structures\",\"type\":\"array\",\"items\":{"
    "\"$ref\":\"#/components/schemas/endpoint_info\"}}},\"400\":{\"$ref\":\"#"
    "/components/responses/400\"}}}},\"/get_sinks\":{\"description\":\"Retrie"
    "ve array of available audio sinks\",\"get\":{\"parameters\":[{\"in\":\"q"
    "uery\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\""
    "string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/20"
    "0\",\"response\":{\"description\":\"Array of endpoint info structures\","
    "\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/endpoint_i"
    "nfo\"}}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/stream_"
    "open\":{\"description\":\"Request opening a stream\",\"get\":{\"x-permis"
    "sions\":{\"$ref\":\"#/components/x-permissions/audiostream\"},\"paramete"
    "rs\":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"sche"
    "ma\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"endpoint_type\""
    ",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"n"
    "ame\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\"}}]"
    ",\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"respo"
    "nse\":{\"description\":\"Stream information structure\",\"$ref\":\"#/com"
    "ponents/schemas/stream_info\"}},\"400\":{\"$ref\":\"#/components/respons"
    "es/400\"}}}},\"/stream_close\":{\"description\":\"Request closing a stre"
    "am\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/a"
    "udiostream\"},\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\",\""
    "required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{"
    "\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/component"
    "s/responses/400\"}}}},\"/set_stream_state\":{\"description\":\"Change st"
    "ream active state\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/"
    "x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query\",\"name\""
    ":\"stream_id\",\"required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":"
    "\"query\",\"name\":\"state\",\"required\":true,\"schema\":{\"type\":\"in"
    "t\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},"
    "\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_stream_mute\""
    ":{\"description\":\"Change stream mute state\",\"get\":{\"x-permissions\""
    ":{\"$ref\":\"#/components/x-permissions/streamcontrol\"},\"parameters\":"
    "[{\"in\":\"query\",\"name\":\"stream_id\",\"required\":true,\"schema\":{"
    "\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"mute\",\"required\":true"
    ",\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/co"
    "mponents/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400"
    "\"}}}},\"/get_stream_info\":{\"description\":\"Retrieve stream informati"
    "on\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\",\""
    "required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{"
    "\"$ref\":\"#/components/responses/200\",\"response\":{\"description\":\""
    "Stream information structure\",\"$ref\":\"#/components/schemas/stream_in"
    "fo\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_volum"
    "e\":{\"description\":\"Set volume on endpoint\",\"get\":{\"x-permissions"
    "\":{\"$ref\":\"#/components/x-permissions/endpointcontrol\"},\"parameter"
    "s\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\"sc"
    "hema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\""
    "required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\""
    ":\"volume\",\"required\":true,\"schema\":{\"type\":\"string\"}}],\"respo"
    "nses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$re"
    "f\":\"#/components/responses/400\"}}}},\"/get_volume\":{\"description\":"
    "\"Get endpoint volume\",\"get\":{\"parameters\":[{\"in\":\"query\",\"nam"
    "e\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\"}},"
    "{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"schema\":"
    "{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/res"
    "ponses/200\",\"response\":{\"description\":\"Endpoint volume value\",\"t"
    "ype\":\"double\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},"
    "\"/get_endpoint_info\":{\"description\":\"Retrieve endpoint information "
    "including its properties\",\"get\":{\"parameters\":[{\"in\":\"query\",\""
    "name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\""
    "}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":false,\"schem"
    "a\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components"
    "/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\""
    "/set_property\":{\"description\":\"Set endpoint property value\",\"get\""
    ":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/endpointcontr"
    "ol\"},\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"req"
    "uired\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":"
    "\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\"}},{\"in\""
    ":\"query\",\"name\":\"property_name\",\"required\":true,\"schema\":{\"ty"
    "pe\":\"string\"}},{\"in\":\"query\",\"name\":\"value\",\"required\":true"
    ",\"schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#"
    "/components/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/"
    "400\"}}}},\"/get_property\":{\"description\":\"Get endpoint property val"
    "ue\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type"
    "\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\""
    "name\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\"}}"
    ",{\"in\":\"query\",\"name\":\"property_name\",\"required\":true,\"schema"
    "\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/componen"
    "ts/responses/200\",\"response\":{\"description\":\"Property value\",\"ty"
    "pe\":\"double\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\""
    "/get_list_actions\":{\"description\":\"Retrieve a list of supported acti"
    "ons for a particular audio role\",\"get\":{\"parameters\":[{\"in\":\"que"
    "ry\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\"st"
    "ring\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\""
    "},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/post_action\":"
    "{\"description\":\"Post sound or audio device related action event (exte"
    "ndable mechanism)\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/"
    "x-permissions/soundevent\"},\"parameters\":[{\"in\":\"query\",\"name\":\""
    "action_name\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\""
    ":\"query\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\""
    ":\"string\"}},{\"in\":\"query\",\"name\":\"media_name\",\"required\":fal"
    "se,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"action"
    "_context\",\"required\":false,\"schema\":{\"type\":\"object\"}}],\"respo"
    "nses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$re"
    "f\":\"#/components/responses/400\"}}}},\"/subscribe\":{\"description\":\""
    "Subscribe to audio high level events\",\"get\":{\"parameters\":[{\"in\":"
    "\"query\",\"name\":\"events\",\"required\":true,\"schema\":{\"type\":\"a"
    "rray\",\"items\":{\"type\":\"string\"}}}],\"responses\":{\"200\":{\"$ref"
    "\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/components/resp"
    "onses/400\"}}}},\"/unsubscribe\":{\"description\":\"Unubscribe to audio "
    "high level events\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":"
    "\"events\",\"required\":true,\"schema\":{\"type\":\"array\",\"items\":{\""
    "type\":\"string\"}}}],\"responses\":{\"200\":{\"$ref\":\"#/components/re"
    "sponses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}}}}"
;

static const struct afb_auth _afb_auths_v2_ahl_4a[] = {
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:audiostream" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:streamcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:endpointcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:soundevent" }
};

 void audiohlapi_get_sources(struct afb_req req);
 void audiohlapi_get_sinks(struct afb_req req);
 void audiohlapi_stream_open(struct afb_req req);
 void audiohlapi_stream_close(struct afb_req req);
 void audiohlapi_set_stream_state(struct afb_req req);
 void audiohlapi_set_stream_mute(struct afb_req req);
 void audiohlapi_get_stream_info(struct afb_req req);
 void audiohlapi_set_volume(struct afb_req req);
 void audiohlapi_get_volume(struct afb_req req);
 void audiohlapi_get_endpoint_info(struct afb_req req);
 void audiohlapi_set_property(struct afb_req req);
 void audiohlapi_get_property(struct afb_req req);
 void audiohlapi_get_list_actions(struct afb_req req);
 void audiohlapi_post_action(struct afb_req req);
 void audiohlapi_subscribe(struct afb_req req);
 void audiohlapi_unsubscribe(struct afb_req req);

static const struct afb_verb_v2 _afb_verbs_v2_ahl_4a[] = {
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
        .auth = &_afb_auths_v2_ahl_4a[0],
        .info = "Request opening a stream",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "stream_close",
        .callback = audiohlapi_stream_close,
        .auth = &_afb_auths_v2_ahl_4a[0],
        .info = "Request closing a stream",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_stream_state",
        .callback = audiohlapi_set_stream_state,
        .auth = &_afb_auths_v2_ahl_4a[1],
        .info = "Change stream active state",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_stream_mute",
        .callback = audiohlapi_set_stream_mute,
        .auth = &_afb_auths_v2_ahl_4a[1],
        .info = "Change stream mute state",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_stream_info",
        .callback = audiohlapi_get_stream_info,
        .auth = NULL,
        .info = "Retrieve stream information",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_volume",
        .callback = audiohlapi_set_volume,
        .auth = &_afb_auths_v2_ahl_4a[2],
        .info = "Set volume on endpoint",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_volume",
        .callback = audiohlapi_get_volume,
        .auth = NULL,
        .info = "Get endpoint volume",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_endpoint_info",
        .callback = audiohlapi_get_endpoint_info,
        .auth = NULL,
        .info = "Retrieve endpoint information including its properties",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_property",
        .callback = audiohlapi_set_property,
        .auth = &_afb_auths_v2_ahl_4a[2],
        .info = "Set endpoint property value",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_property",
        .callback = audiohlapi_get_property,
        .auth = NULL,
        .info = "Get endpoint property value",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "get_list_actions",
        .callback = audiohlapi_get_list_actions,
        .auth = NULL,
        .info = "Retrieve a list of supported actions for a particular audio role",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "post_action",
        .callback = audiohlapi_post_action,
        .auth = &_afb_auths_v2_ahl_4a[3],
        .info = "Post sound or audio device related action event (extendable mechanism)",
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
    .api = "ahl-4a",
    .specification = _afb_description_v2_ahl_4a,
    .info = "Audio high level API for AGL applications",
    .verbs = _afb_verbs_v2_ahl_4a,
    .preinit = NULL,
    .init = AhlBindingInit,
    .onevent = AhlOnEvent,
    .noconcurrency = 0
};

