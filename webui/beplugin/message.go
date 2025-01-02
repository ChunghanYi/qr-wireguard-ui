package beplugin

//WebUI -> backend daemon
type RequestMessage struct {
	Cmd          string     `cmd:=HELLO`
	SubCmd       string     `subcmd:=wg_peer_add`
	FieldCount   string     `field_count:=N`
	KeyValue     [16]string `keyN:=valueN`
}

//WebUI <- backend daemon
type ReplyMessage struct {
	Cmd          string    `cmd:=OK`
}
