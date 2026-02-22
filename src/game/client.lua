local client = nil

function game_init()
    client = core.client.new("127.0.0.1", 7777)
end

function game_update(dt)
    local ev = client:poll()
    while ev do
        if ev.type == core.net_event.connect then
            core.print("connected, id=" .. ev.id)
            client:send("hello from " .. ev.id)
        elseif ev.type == core.net_event.disconnect then
            core.print("disconnected")
        elseif ev.type == core.net_event.data then
            core.print("got: " .. ev.data)
        end
        ev = client:poll()
    end
end

function game_quit()
    if client then client:close() end
end