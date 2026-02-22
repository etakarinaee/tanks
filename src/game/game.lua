
function game_init()
end

function game_update(delta_time)
    core.push_quad({0.1, 0.1}, {0.5, 0.5, 0.5});

    if core.key_pressed(key.s) == 1 then 
        core.print("s pressed")
    end
end

function game_quit()
end
