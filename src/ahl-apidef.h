
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
    ":{\"/get_sources\":{\"description\":\"Retrieve array of available audio "
    "sources\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"audio_ro"
    "le\",\"required\":true,\"schema\":{\"type\":\"string\"}}],\"responses\":"
    "{\"200\":{\"$ref\":\"#/components/responses/200\",\"response\":{\"descri"
    "ption\":\"Array of endpoint info structures\",\"type\":\"array\",\"items"
    "\":{\"$ref\":\"#/components/schemas/endpoint_info\"}}},\"400\":{\"$ref\""
    ":\"#/components/responses/400\"}}}},\"/get_sinks\":{\"description\":\"Re"
    "trieve array of available audio sinks\",\"get\":{\"parameters\":[{\"in\""
    ":\"query\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\""
    ":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses"
    "/200\",\"response\":{\"description\":\"Array of endpoint info structures"
    "\",\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/endpoin"
    "t_info\"}}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/stre"
    "am_open\":{\"description\":\"Request opening a stream\",\"get\":{\"x-per"
    "missions\":{\"$ref\":\"#/components/x-permissions/audiostream\"},\"param"
    "eters\":[{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"s"
    "chema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"endpoint_typ"
    "e\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\","
    "\"name\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\""
    "}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"re"
    "sponse\":{\"description\":\"Stream information structure\",\"$ref\":\"#/"
    "components/schemas/stream_info\"}},\"400\":{\"$ref\":\"#/components/resp"
    "onses/400\"}}}},\"/stream_close\":{\"description\":\"Request closing a s"
    "tream\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permission"
    "s/audiostream\"},\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id\""
    ",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\""
    ":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/compone"
    "nts/responses/400\"}}}},\"/set_stream_state\":{\"description\":\"Change "
    "stream active state\",\"get\":{\"x-permissions\":{\"$ref\":\"#/component"
    "s/x-permissions/streamcontrol\"},\"parameters\":[{\"in\":\"query\",\"nam"
    "e\":\"stream_id\",\"required\":true,\"schema\":{\"type\":\"int\"}},{\"in"
    "\":\"query\",\"name\":\"state\",\"required\":true,\"schema\":{\"type\":\""
    "int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\""
    "},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_stream_mut"
    "e\":{\"description\":\"Change stream mute state\",\"get\":{\"x-permissio"
    "ns\":{\"$ref\":\"#/components/x-permissions/streamcontrol\"},\"parameter"
    "s\":[{\"in\":\"query\",\"name\":\"stream_id\",\"required\":true,\"schema"
    "\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"mute\",\"required\":"
    "true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\""
    "#/components/responses/200\"},\"400\":{\"$ref\":\"#/components/responses"
    "/400\"}}}},\"/get_stream_info\":{\"description\":\"Retrieve stream infor"
    "mation\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"stream_id"
    "\",\"required\":true,\"schema\":{\"type\":\"int\"}}],\"responses\":{\"20"
    "0\":{\"$ref\":\"#/components/responses/200\",\"response\":{\"description"
    "\":\"Stream information structure\",\"$ref\":\"#/components/schemas/stre"
    "am_info\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/set_"
    "volume\":{\"description\":\"Set volume on endpoint\",\"get\":{\"x-permis"
    "sions\":{\"$ref\":\"#/components/x-permissions/endpointcontrol\"},\"para"
    "meters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":true"
    ",\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\"endpoint_i"
    "d\",\"required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\""
    "name\":\"volume\",\"required\":true,\"schema\":{\"type\":\"string\"}}],\""
    "responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{"
    "\"$ref\":\"#/components/responses/400\"}}}},\"/get_volume\":{\"descripti"
    "on\":\"Get endpoint volume\",\"get\":{\"parameters\":[{\"in\":\"query\","
    "\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum"
    "\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"sche"
    "ma\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/component"
    "s/responses/200\",\"response\":{\"description\":\"Endpoint volume value\""
    ",\"type\":\"double\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}"
    "}}},\"/get_endpoint_info\":{\"description\":\"Retrieve endpoint informat"
    "ion including its properties\",\"get\":{\"parameters\":[{\"in\":\"query\""
    ",\"name\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enu"
    "m\"}},{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":false,\"sc"
    "hema\":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/compone"
    "nts/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}"
    "},\"/set_property\":{\"description\":\"Set endpoint property value\",\"g"
    "et\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/endpointc"
    "ontrol\"},\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\""
    "required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name"
    "\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"int\"}},{\""
    "in\":\"query\",\"name\":\"property_name\",\"required\":true,\"schema\":{"
    "\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"value\",\"required\":"
    "true,\"schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\""
    ":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/components/respon"
    "ses/400\"}}}},\"/get_property\":{\"description\":\"Get endpoint property"
    " value\",\"get\":{\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_"
    "type\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query"
    "\",\"name\":\"endpoint_id\",\"required\":false,\"schema\":{\"type\":\"in"
    "t\"}},{\"in\":\"query\",\"name\":\"property_name\",\"required\":true,\"s"
    "chema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/com"
    "ponents/responses/200\",\"response\":{\"description\":\"Property value\""
    ",\"type\":\"double\"}},\"400\":{\"$ref\":\"#/components/responses/400\"}"
    "}}},\"/get_list_actions\":{\"description\":\"Retrieve a list of supporte"
    "d actions for a particular audio role\",\"get\":{\"parameters\":[{\"in\""
    ":\"query\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\""
    ":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses"
    "/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/post_act"
    "ion\":{\"description\":\"Post sound or audio device related action event"
    " (extendable mechanism)\",\"get\":{\"x-permissions\":{\"$ref\":\"#/compo"
    "nents/x-permissions/soundevent\"},\"parameters\":[{\"in\":\"query\",\"na"
    "me\":\"action_name\",\"required\":true,\"schema\":{\"type\":\"string\"}}"
    ",{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"schema\":"
    "{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"media_name\",\"requi"
    "red\":false,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\""
    ":\"action_context\",\"required\":false,\"schema\":{\"type\":\"object\"}}"
    "],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400"
    "\":{\"$ref\":\"#/components/responses/400\"}}}},\"/subscribe\":{\"descri"
    "ption\":\"Subscribe to audio high level events\",\"get\":{\"parameters\""
    ":[{\"in\":\"query\",\"name\":\"events\",\"required\":true,\"schema\":{\""
    "type\":\"array\",\"items\":{\"type\":\"string\"}}}],\"responses\":{\"200"
    "\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#/compo"
    "nents/responses/400\"}}}},\"/unsubscribe\":{\"description\":\"Unubscribe"
    " to audio high level events\",\"get\":{\"parameters\":[{\"in\":\"query\""
    ",\"name\":\"events\",\"required\":true,\"schema\":{\"type\":\"array\",\""
    "items\":{\"type\":\"string\"}}}],\"responses\":{\"200\":{\"$ref\":\"#/co"
    "mponents/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400"
    "\"}}}}}}"
;

static const struct afb_auth _afb_auths_v2_audiohl[] = {
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
        .verb = "set_stream_state",
        .callback = audiohlapi_set_stream_state,
        .auth = &_afb_auths_v2_audiohl[1],
        .info = "Change stream active state",
        .session = AFB_SESSION_NONE_V2
    },
    {
        .verb = "set_stream_mute",
        .callback = audiohlapi_set_stream_mute,
        .auth = &_afb_auths_v2_audiohl[1],
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
        .auth = &_afb_auths_v2_audiohl[2],
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
        .auth = &_afb_auths_v2_audiohl[2],
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
        .auth = &_afb_auths_v2_audiohl[3],
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
    .api = "audiohl",
    .specification = _afb_description_v2_audiohl,
    .info = "Audio high level API for AGL applications",
    .verbs = _afb_verbs_v2_audiohl,
    .preinit = NULL,
    .init = AhlBindingInit,
    .onevent = AhlOnEvent,
    .noconcurrency = 0
};

