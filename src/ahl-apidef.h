
static const char _afb_description_v2_audiohl[] =
    "{\"openapi\":\"3.0.0\",\"info\":{\"description\":\"Audio high level API "
    "for AGL applications\",\"title\":\"audiohighlevel\",\"version\":\"1.0\","
    "\"x-binding-c-generator\":{\"api\":\"audiohl\",\"version\":2,\"prefix\":"
    "\"audiohlapi_\",\"postfix\":\"\",\"start\":null,\"onevent\":\"AhlOnEvent"
    "\",\"init\":\"AhlBindingInit\",\"scope\":\"\",\"private\":false}},\"serv"
    "ers\":[{\"url\":\"ws://{host}:{port}/api/audiohl\",\"description\":\"Aud"
    "io high level API for AGL applications.\",\"variables\":{\"host\":{\"def"
    "ault\":\"localhost\"},\"port\":{\"default\":\"1234\"}},\"x-afb-events\":"
    "[{\"$ref\":\"#/components/schemas/afb-event\"}]}],\"components\":{\"sche"
    "mas\":{\"afb-reply\":{\"$ref\":\"#/components/schemas/afb-reply-v2\"},\""
    "afb-event\":{\"$ref\":\"#/components/schemas/afb-event-v2\"},\"afb-reply"
    "-v2\":{\"title\":\"Generic response.\",\"type\":\"object\",\"required\":"
    "[\"jtype\",\"request\"],\"properties\":{\"jtype\":{\"type\":\"string\",\""
    "const\":\"afb-reply\"},\"request\":{\"type\":\"object\",\"required\":[\""
    "status\"],\"properties\":{\"status\":{\"type\":\"string\"},\"info\":{\"t"
    "ype\":\"string\"},\"token\":{\"type\":\"string\"},\"uuid\":{\"type\":\"s"
    "tring\"},\"reqid\":{\"type\":\"string\"}}},\"response\":{\"type\":\"obje"
    "ct\"}}},\"afb-event-v2\":{\"type\":\"object\",\"required\":[\"jtype\",\""
    "event\"],\"properties\":{\"jtype\":{\"type\":\"string\",\"const\":\"afb-"
    "event\"},\"event\":{\"type\":\"string\"},\"data\":{\"type\":\"object\"}}"
    "},\"endpoint_info\":{\"type\":\"object\",\"required\":[\"endpoint_id\",\""
    "type\",\"device_name\",\"device_uri\"],\"properties\":{\"endpoint_id\":{"
    "\"type\":\"int\"},\"type\":{\"type\":\"enum\"},\"device_name\":{\"type\""
    ":\"string\"},\"device_uri_type\":{\"type\":\"string\"}}},\"stream_info\""
    ":{\"type\":\"object\",\"required\":[\"stream_id\",\"state\",\"mute\",\"e"
    "ndpoint_info\"],\"properties\":{\"stream_id\":{\"type\":\"int\"},\"state"
    "\":{\"type\":\"int\"},\"mute\":{\"type\":\"int\"},\"device_uri\":{\"type"
    "\":\"string\"},\"$ref\":\"#/components/schemas/endpoint_info\"}}},\"x-pe"
    "rmissions\":{\"streamcontrol\":{\"permission\":\"urn:AGL:permission:audi"
    "o:public:streamcontrol\"},\"endpointcontrol\":{\"permission\":\"urn:AGL:"
    "permission:audio:public:endpointcontrol\"},\"audiostream\":{\"permission"
    "\":\"urn:AGL:permission:audio:public:audiostream\"},\"soundevent\":{\"pe"
    "rmission\":\"urn:AGL:permission:audio:public:soundevent\"}},\"responses\""
    ":{\"200\":{\"description\":\"A complex object array response\",\"content"
    "\":{\"application/json\":{\"schema\":{\"$ref\":\"#/components/schemas/af"
    "b-reply\"}}}},\"400\":{\"description\":\"Invalid arguments\"}}},\"paths\""
    ":{\"/get_endpoints\":{\"description\":\"Retrieve array of available audi"
    "o endpoints\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"audi"
    "o_role\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"q"
    "uery\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\""
    ":\"enum\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/2"
    "00\",\"response\":{\"description\":\"Array of endpoint info structures\""
    ",\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/endpoint_"
    "info\"}}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/stream"
    "_open\":{\"description\":\"Request opening a stream\",\"get\":{\"x-permi"
    "ssions\":{\"$ref\":\"#/components/x-permissions/audiostream\"},\"paramet"
    "ers\":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"sch"
    "ema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"endpoint_type\""
    ",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"n"
    "ame\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\"}}]"
    ",\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"respo"
    "nse\":{\"description\":\"Stream information structure\",\"$ref\":\"#/com"
    "ponents/schemas/stream_info\"}},\"400\":{\"$ref\":\"#/components/respons"
    "es/400\"}}}},\"/stream_close\":{\"description\":\"Request closing a stre"
    "am\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/a"
    "udiostream\"},\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\",\""
    "required\":false,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":"
    "{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/componen"
    "ts/responses/400\"}}}},\"/set_stream_state\":{\"description\":\"Change s"
    "tream active and/or mute state\",\"get\":{\"x-permissions\":{\"$ref\":\""
    "#/components/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"qu"
    "ery\",\"name\":\"stream_id\",\"required\":false,\"schema\":{\"type\":\"i"
    "nt\"}},{\"in\":\"query\",\"name\":\"state\",\"required\":false,\"schema\""
    ":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"mute\",\"required\":fa"
    "lse,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#"
    "/components/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/"
    "400\"}}}},\"/get_stream_info\":{\"description\":\"Retrieve stream inform"
    "ation\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\""
    ",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\""
    ":{\"$ref\":\"#/components/responses/200\",\"response\":{\"description\":"
    "\"Stream information structure\",\"$ref\":\"#/components/schemas/stream_"
    "info\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/volume\""
    ":{\"description\":\"Set or get volume on endpoint\",\"get\":{\"x-permiss"
    "ions\":{\"$ref\":\"#/components/x-permissions/endpointcontrol\"},\"param"
    "eters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true,"
    "\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id"
    "\",\"required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\""
    "name\":\"volume\",\"required\":false,\"schema\":{\"type\":\"string\"}}],"
    "\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\""
    ":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_endpoint_info\":{\""
    "description\":\"Retrieve endpoint information including its properties\""
    ",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\""
    "required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name"
    "\":\"endpoint_id\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"r"
    "esponses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\""
    "$ref\":\"#/components/responses/400\"}}}},\"/property\":{\"description\""
    ":\"Set/get endpoint property value\",\"get\":{\"x-permissions\":{\"$ref\""
    ":\"#/components/x-permissions/endpointcontrol\"},\"parameters\":[{\"in\""
    ":\"query\",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"ty"
    "pe\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":"
    "true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"propert"
    "y_name\",\"required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"q"
    "uery\",\"name\":\"value\",\"required\":false,\"schema\":{\"type\":\"stri"
    "ng\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"}"
    ",\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/get_list_action"
    "s\":{\"description\":\"Retrieve a list of supported actions for a partic"
    "ular audio role\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\""
    "audio_role\",\"required\":true,\"schema\":{\"type\":\"string\"}}],\"resp"
    "onses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$r"
    "ef\":\"#/components/responses/400\"}}}},\"/post_action\":{\"description\""
    ":\"Post sound or audio device related action event (extendable mechanism"
    ")\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/so"
    "undevent\"},\"parameters\":[{\"in\":\"query\",\"name\":\"action_name\",\""
    "required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"na"
    "me\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\"string\"}},"
    "{\"in\":\"query\",\"name\":\"media_name\",\"required\":false,\"schema\":"
    "{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"action_context\",\"r"
    "equired\":false,\"schema\":{\"type\":\"object\"}}],\"responses\":{\"200\""
    ":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/compone"
    "nts/responses/400\"}}}},\"/event_subscription\":{\"description\":\"Subsc"
    "ribe to audio high level events\",\"get\":{\"parameters\":[{\"in\":\"que"
    "ry\",\"name\":\"events\",\"required\":true,\"schema\":{\"type\":\"array\""
    ",\"items\":{\"type\":\"string\"}}},{\"in\":\"query\",\"name\":\"subscrib"
    "e\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"2"
    "00\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/com"
    "ponents/responses/400\"}}}}}}"
;

static const struct afb_auth _afb_auths_v2_audiohl[] = {
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

static const struct afb_verb_v2 _afb_verbs_v2_audiohl[] = {
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
        .verb = "set_stream_state",
        .callback = audiohlapi_set_stream_state,
        .auth = &_afb_auths_v2_audiohl[1],
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
        .auth = &_afb_auths_v2_audiohl[2],
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
        .auth = &_afb_auths_v2_audiohl[2],
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
        .auth = &_afb_auths_v2_audiohl[3],
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
    .api = "audiohl",
    .specification = _afb_description_v2_audiohl,
    .info = "Audio high level API for AGL applications",
    .verbs = _afb_verbs_v2_audiohl,
    .preinit = NULL,
    .init = AhlBindingInit,
    .onevent = AhlOnEvent,
    .noconcurrency = 0
};

