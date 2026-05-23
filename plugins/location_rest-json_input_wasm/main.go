package main

import (
	"encoding/json"
	"fmt"
	"unsafe"
)

// PluginState holds the runtime information
type PluginState struct {
	Topic         string
	Version       string
	InterfaceType string
	RestURL       string
	LastResult    []byte
}

var state = PluginState{
	Topic:         "location",
	Version:       "1.2.0",
	InterfaceType: "rest-json",
}

// --- Host Function Imports ---

//go:wasmimport env host_http_get
func host_http_get(urlPtr *byte, urlLen int32) *byte

//go:wasmimport env host_http_get_len
func host_http_get_len() int32

// --- Plugin Exports ---

//export allocate
func allocate(size int32) *byte {
	buf := make([]byte, size)
	return &buf[0]
}

//export deallocate
func deallocate(ptr *byte, size int32) {}

//export get_last_result_len
func get_last_result_len() int32 {
	return int32(len(state.LastResult))
}

func returnString(s string) *byte {
	state.LastResult = []byte(s)
	if len(state.LastResult) == 0 {
		return nil
	}
	return &state.LastResult[0]
}

//export get_topic
func get_topic() *byte {
	return returnString(state.Topic)
}

//export get_version
func get_version() *byte {
	return returnString(state.Version)
}

//export get_interface_type
func get_interface_type() *byte {
	return returnString(state.InterfaceType)
}

//export initialize
func initialize(ptr *byte, length int32) int32 {
	configBytes := unsafe.Slice(ptr, length)
	var config map[string]interface{}
	if err := json.Unmarshal(configBytes, &config); err != nil {
		return 0
	}

	// Use connection_string (passed from data_rest-json in INI)
	if url, ok := config["connection_string"].(string); ok {
		state.RestURL = url
	}
	
	if t, ok := config["topic"].(string); ok {
		state.Topic = t
	}

	return 1
}

//export fetch_batch
func fetch_batch(maxRecords int32) *byte {
	if state.RestURL == "" {
		return returnString(`[{"error": "No REST URL configured"}]`)
	}

	// Prepare URL for host call
	urlBytes := []byte(state.RestURL)
	urlPtr := &urlBytes[0]
	urlLen := int32(len(urlBytes))

	// Call host-assisted HTTP fetch
	respPtr := host_http_get(urlPtr, urlLen)
	if respPtr == nil {
		return returnString(`[{"error": "Host HTTP fetch failed"}]`)
	}

	respLen := host_http_get_len()
	respBytes := unsafe.Slice(respPtr, respLen)

	// Validate and potentially wrap JSON
	var data interface{}
	if err := json.Unmarshal(respBytes, &data); err != nil {
		return returnString(fmt.Sprintf(`[{"error": "Invalid JSON from API: %v"}]`, err))
	}

	// If API returns a single object, wrap it in an array for the Core
	if _, ok := data.(map[string]interface{}); ok {
		wrapped, _ := json.Marshal([]interface{}{data})
		state.LastResult = wrapped
	} else {
		state.LastResult = respBytes
	}

	return &state.LastResult[0]
}

//export shutdown
func shutdown() {}

func main() {}
