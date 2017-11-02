
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
    "/get_endpoints\":{\"description\":\"Retrieve array of available audio en"
    "dpoints\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"audio_ro"
    "le\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query"
    "\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"e"
    "num\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\""
    ",\"response\":{\"description\":\"Array of endpoint info structures\",\"t"
    "ype\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/endpoint_info"
    "\"}}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/stream_ope"
    "n\":{\"description\":\"Request opening a stream\",\"get\":{\"x-permissio"
    "ns\":{\"$ref\":\"#/components/x-permissions/audiostream\"},\"parameters\""
    ":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"schema\""
    ":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"endpoint_type\",\"r"
    "equired\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\""
    ":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\"}}],\"re"
    "sponses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"response\""
    ":{\"description\":\"Stream information structure\",\"$ref\":\"#/componen"
    "ts/schemas/stream_info\"}},\"400\":{\"$ref\":\"#/components/responses/40"
    "0\"}}}},\"/stream_close\":{\"description\":\"Request closing a stream\","
    "\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/audios"
    "tream\"},\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\",\"requ"
    "ired\":false,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$"
    "ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/components/r"
    "esponses/400\"}}}},\"/set_stream_state\":{\"description\":\"Change strea"
    "m active and/or mute state\",\"get\":{\"x-permissions\":{\"$ref\":\"#/co"
    "mponents/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query\""
    ",\"name\":\"stream_id\",\"required\":false,\"schema\":{\"type\":\"int\"}"
    "},{\"in\":\"query\",\"name\":\"state\",\"required\":false,\"schema\":{\""
    "type\":\"int\"}},{\"in\":\"query\",\"name\":\"mute\",\"required\":false,"
    "\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/com"
    "ponents/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\""
    "}}}},\"/get_stream_info\":{\"description\":\"Retrieve stream information"
    "\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\",\"r"
    "equired\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\""
    "$ref\":\"#/components/responses/200\",\"response\":{\"description\":\"St"
    "ream information structure\",\"$ref\":\"#/components/schemas/stream_info"
    "\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/volume\":{\""
    "description\":\"Set or get volume on endpoint\",\"get\":{\"x-permissions"
    "\":{\"$ref\":\"#/components/x-permissions/endpointcontrol\"},\"parameter"
    "s\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true,\"sc"
    "hema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\""
    "required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\""
    ":\"volume\",\"required\":false,\"schema\":{\"type\":\"string\"}}],\"resp"
    "onses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$r"
    "ef\":\"#/components/responses/400\"}}}},\"/get_endpoint_info\":{\"descri"
    "ption\":\"Retrieve endpoint information including its properties\",\"get"
    "\":{\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"requi"
    "red\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\""
    "endpoint_id\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"respon"
    "ses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref"
    "\":\"#/components/responses/400\"}}}},\"/property\":{\"description\":\"S"
    "et/get endpoint property value\",\"get\":{\"x-permissions\":{\"$ref\":\""
    "#/components/x-permissions/endpointcontrol\"},\"parameters\":[{\"in\":\""
    "query\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\""
    ":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true"
    ",\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"property_na"
    "me\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query"
    "\",\"name\":\"value\",\"required\":false,\"schema\":{\"type\":\"string\""
    "}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"4"
    "00\":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_list_actions\":"
    "{\"description\":\"Retrieve a list of supported actions for a particular"
    " audio role\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"audi"
    "o_role\",\"required\":true,\"schema\":{\"type\":\"string\"}}],\"response"
    "s\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\""
    ":\"#/components/responses/400\"}}}},\"/post_action\":{\"description\":\""
    "Post sound or audio device related action event (extendable mechanism)\""
    ",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/sound"
    "event\"},\"parameters\":[{\"in\":\"query\",\"name\":\"action_name\",\"re"
    "quired\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name"
    "\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\""
    "in\":\"query\",\"name\":\"media_name\",\"required\":false,\"schema\":{\""
    "type\":\"string\"}},{\"in\":\"query\",\"name\":\"action_context\",\"requ"
    "ired\":false,\"schema\":{\"type\":\"object\"}}],\"responses\":{\"200\":{"
    "\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/component"
    "s/responses/400\"}}}},\"/event_subscription\":{\"description\":\"Subscri"
    "be to audio high level events\",\"get\":{\"parameters\":[{\"in\":\"query"
    "\",\"name\":\"events\",\"required\":true,\"schema\":{\"type\":\"array\","
    "\"items\":{\"type\":\"string\"}}},{\"in\":\"query\",\"name\":\"subscribe"
    "\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"20"
    "0\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/comp"
    "onents/responses/400\"}}}}}}"
;

static const struct afb_auth _afb_auths_v2_ahl_4a[] = {
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:audiostream" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:streamcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:endpointcontrol" },
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission:audio:public:soundevent" }
};

 void audiohlapi_get_endpoints(struct afb_req req);
 void audiohlapi_stream_open(struct afb_req req);
 void audiohlapi_stream_close(struct afb_req req);
 void audiohlapi_set_stream_state(struct afb_req req);
 void audiohlapi_get_stream_info(struct afb_req req);
 void audiohlapi_volume(struct afb_req req);
 void audiohlapi_get_endpoint_info(struct afb_req req);
 void audiohlapi_property(struct afb_req req);
 void audiohlapi_get_list_actions(struct afb_req req);
 void audiohlapi_post_action(struct afb_req req);
 void audiohlapi_event_subscription(struct afb_req req);

static const struct afb_verb_v2 _afb_verbs_v2_ahl_4a[] = {
    {
        .verb = "get_endpoints",
        .callback = audiohlapi_get_endpoints,
        .auth = NULL,
        .info = "Retrieve array of available audio endpoints",
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
        .info = "Change stream active and/or mute state",
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
        .verb = "volume",
        .callback = audiohlapi_volume,
        .auth = &_afb_auths_v2_ahl_4a[2],
        .info = "Set or get volume on endpoint",
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
        .verb = "property",
        .callback = audiohlapi_property,
        .auth = &_afb_auths_v2_ahl_4a[2],
        .info = "Set/get endpoint property value",
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
        .verb = "event_subscription",
        .callback = audiohlapi_event_subscription,
        .auth = NULL,
        .info = "Subscribe to audio high level events",
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

