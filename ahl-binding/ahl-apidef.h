
static const char _afb_description_v2_ahl_4a[] =
    "{\"openapi\":\"3.0.0\",\"info\":{\"description\":\"Audio high level API "
    "for AGL applications\",\"title\":\"audiohighlevel\",\"version\":\"1.0\","
    "\"x-binding-c-generator\":{\"api\":\"ahl-4a\",\"version\":2,\"prefix\":\""
    "audiohlapi_\",\"postfix\":\"\",\"start\":null,\"onevent\":\"AhlOnEvent\""
    ",\"init\":\"AhlBindingInit\",\"scope\":\"\",\"private\":false,\"noconcur"
    "rency\":false}},\"servers\":[{\"url\":\"ws://{host}:{port}/api/audiohl\""
    ",\"description\":\"Audio high level API for AGL applications.\",\"variab"
    "les\":{\"host\":{\"default\":\"localhost\"},\"port\":{\"default\":\"1234"
    "\"}},\"x-afb-events\":[{\"$ref\":\"#/components/schemas/afb-event\"}]}],"
    "\"components\":{\"schemas\":{\"afb-reply\":{\"$ref\":\"#/components/sche"
    "mas/afb-reply-v2\"},\"afb-event\":{\"$ref\":\"#/components/schemas/afb-e"
    "vent-v2\"},\"afb-reply-v2\":{\"title\":\"Generic response.\",\"type\":\""
    "object\",\"required\":[\"jtype\",\"request\"],\"properties\":{\"jtype\":"
    "{\"type\":\"string\",\"const\":\"afb-reply\"},\"request\":{\"type\":\"ob"
    "ject\",\"required\":[\"status\"],\"properties\":{\"status\":{\"type\":\""
    "string\"},\"info\":{\"type\":\"string\"},\"token\":{\"type\":\"string\"}"
    ",\"uuid\":{\"type\":\"string\"},\"reqid\":{\"type\":\"string\"}}},\"resp"
    "onse\":{\"type\":\"object\"}}},\"afb-event-v2\":{\"type\":\"object\",\"r"
    "equired\":[\"jtype\",\"event\"],\"properties\":{\"jtype\":{\"type\":\"st"
    "ring\",\"const\":\"afb-event\"},\"event\":{\"type\":\"string\"},\"data\""
    ":{\"type\":\"object\"}}},\"endpoint_info\":{\"type\":\"object\",\"requir"
    "ed\":[\"endpoint_id\",\"type\",\"device_name\",\"device_uri\"],\"propert"
    "ies\":{\"endpoint_id\":{\"type\":\"int\"},\"type\":{\"type\":\"enum\"},\""
    "device_name\":{\"type\":\"string\"},\"device_uri_type\":{\"type\":\"stri"
    "ng\"}}},\"stream_info\":{\"type\":\"object\",\"required\":[\"stream_id\""
    ",\"state\",\"mute\",\"endpoint_info\"],\"properties\":{\"stream_id\":{\""
    "type\":\"int\"},\"state\":{\"type\":\"int\"},\"mute\":{\"type\":\"int\"}"
    ",\"device_uri\":{\"type\":\"string\"},\"$ref\":\"#/components/schemas/en"
    "dpoint_info\"}}},\"x-permissions\":{\"streamcontrol\":{\"permission\":\""
    "urn:AGL:permission:audio:public:streamcontrol\"},\"endpointcontrol\":{\""
    "permission\":\"urn:AGL:permission:audio:public:endpointcontrol\"},\"audi"
    "ostream\":{\"permission\":\"urn:AGL:permission:audio:public:audiostream\""
    "},\"soundevent\":{\"permission\":\"urn:AGL:permission:audio:public:sound"
    "event\"}},\"responses\":{\"200\":{\"description\":\"A complex object arr"
    "ay response\",\"content\":{\"application/json\":{\"schema\":{\"$ref\":\""
    "#/components/schemas/afb-reply\"}}}},\"400\":{\"description\":\"Invalid "
    "arguments\"}}},\"paths\":{\"/get_endpoints\":{\"description\":\"Retrieve"
    " array of available audio endpoints\",\"get\":{\"parameters\":[{\"in\":\""
    "query\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"type\":\""
    "string\"}},{\"in\":\"query\",\"name\":\"endpoint_type\",\"required\":tru"
    "e,\"schema\":{\"type\":\"enum\"}}],\"responses\":{\"200\":{\"$ref\":\"#/"
    "components/responses/200\",\"response\":{\"description\":\"Array of endp"
    "oint info structures\",\"type\":\"array\",\"items\":{\"$ref\":\"#/compon"
    "ents/schemas/endpoint_info\"}}},\"400\":{\"$ref\":\"#/components/respons"
    "es/400\"}}}},\"/stream_open\":{\"description\":\"Request opening a strea"
    "m\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/au"
    "diostream\"},\"parameters\":[{\"in\":\"query\",\"name\":\"audio_role\",\""
    "required\":true,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"na"
    "me\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\"}}"
    ",{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":false,\"schema\""
    ":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/re"
    "sponses/200\",\"response\":{\"description\":\"Stream information structu"
    "re\",\"$ref\":\"#/components/schemas/stream_info\"}},\"400\":{\"$ref\":\""
    "#/components/responses/400\"}}}},\"/stream_close\":{\"description\":\"Re"
    "quest closing a stream\",\"get\":{\"x-permissions\":{\"$ref\":\"#/compon"
    "ents/x-permissions/audiostream\"},\"parameters\":[{\"in\":\"query\",\"na"
    "me\":\"stream_id\",\"required\":false,\"schema\":{\"type\":\"int\"}}],\""
    "responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{"
    "\"$ref\":\"#/components/responses/400\"}}}},\"/set_stream_state\":{\"des"
    "cription\":\"Change stream active and/or mute state\",\"get\":{\"x-permi"
    "ssions\":{\"$ref\":\"#/components/x-permissions/streamcontrol\"},\"param"
    "eters\":[{\"in\":\"query\",\"name\":\"stream_id\",\"required\":false,\"s"
    "chema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\"state\",\"requi"
    "red\":false,\"schema\":{\"type\":\"int\"}},{\"in\":\"query\",\"name\":\""
    "mute\",\"required\":false,\"schema\":{\"type\":\"int\"}}],\"responses\":"
    "{\"200\":{\"$ref\":\"#/components/responses/200\"},\"400\":{\"$ref\":\"#"
    "/components/responses/400\"}}}},\"/get_stream_info\":{\"description\":\""
    "Retrieve stream information\",\"get\":{\"parameters\":[{\"in\":\"query\""
    ",\"name\":\"stream_id\",\"required\":true,\"schema\":{\"type\":\"int\"}}"
    "],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\",\"resp"
    "onse\":{\"description\":\"Stream information structure\",\"$ref\":\"#/co"
    "mponents/schemas/stream_info\"}},\"400\":{\"$ref\":\"#/components/respon"
    "ses/400\"}}}},\"/volume\":{\"description\":\"Set or get volume on endpoi"
    "nt\",\"get\":{\"x-permissions\":{\"$ref\":\"#/components/x-permissions/e"
    "ndpointcontrol\"},\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_"
    "type\",\"required\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query"
    "\",\"name\":\"endpoint_id\",\"required\":true,\"schema\":{\"type\":\"int"
    "\"}},{\"in\":\"query\",\"name\":\"volume\",\"required\":false,\"schema\""
    ":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components"
    "/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\""
    "/get_endpoint_info\":{\"description\":\"Retrieve endpoint information in"
    "cluding its properties\",\"get\":{\"parameters\":[{\"in\":\"query\",\"na"
    "me\":\"endpoint_type\",\"required\":true,\"schema\":{\"type\":\"enum\"}}"
    ",{\"in\":\"query\",\"name\":\"endpoint_id\",\"required\":true,\"schema\""
    ":{\"type\":\"int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/re"
    "sponses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/p"
    "roperty\":{\"description\":\"Set/get endpoint property value\",\"get\":{"
    "\"x-permissions\":{\"$ref\":\"#/components/x-permissions/endpointcontrol"
    "\"},\"parameters\":[{\"in\":\"query\",\"name\":\"endpoint_type\",\"requi"
    "red\":true,\"schema\":{\"type\":\"enum\"}},{\"in\":\"query\",\"name\":\""
    "endpoint_id\",\"required\":true,\"schema\":{\"type\":\"int\"}},{\"in\":\""
    "query\",\"name\":\"property_name\",\"required\":true,\"schema\":{\"type\""
    ":\"string\"}},{\"in\":\"query\",\"name\":\"value\",\"required\":false,\""
    "schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/co"
    "mponents/responses/200\"},\"400\":{\"$ref\":\"#/components/responses/400"
    "\"}}}},\"/get_list_actions\":{\"description\":\"Retrieve a list of suppo"
    "rted actions for a particular audio role\",\"get\":{\"parameters\":[{\"i"
    "n\":\"query\",\"name\":\"audio_role\",\"required\":true,\"schema\":{\"ty"
    "pe\":\"string\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/respo"
    "nses/200\"},\"400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/post"
    "_action\":{\"description\":\"Post sound or audio device related action e"
    "vent (extendable mechanism)\",\"get\":{\"x-permissions\":{\"$ref\":\"#/c"
    "omponents/x-permissions/soundevent\"},\"parameters\":[{\"in\":\"query\","
    "\"name\":\"action_name\",\"required\":true,\"schema\":{\"type\":\"string"
    "\"}},{\"in\":\"query\",\"name\":\"audio_role\",\"required\":true,\"schem"
    "a\":{\"type\":\"string\"}},{\"in\":\"query\",\"name\":\"media_name\",\"r"
    "equired\":false,\"schema\":{\"type\":\"string\"}},{\"in\":\"query\",\"na"
    "me\":\"action_context\",\"required\":false,\"schema\":{\"type\":\"object"
    "\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\"},\""
    "400\":{\"$ref\":\"#/components/responses/400\"}}}},\"/event_subscription"
    "\":{\"description\":\"Subscribe to audio high level events\",\"get\":{\""
    "parameters\":[{\"in\":\"query\",\"name\":\"events\",\"required\":true,\""
    "schema\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}},{\"in\":\""
    "query\",\"name\":\"subscribe\",\"required\":true,\"schema\":{\"type\":\""
    "int\"}}],\"responses\":{\"200\":{\"$ref\":\"#/components/responses/200\""
    "},\"400\":{\"$ref\":\"#/components/responses/400\"}}}}}}"
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

