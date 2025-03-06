local function init_array(m, n, fill)
    local mat = {}

    local fill_func;
    if fill then
        fill_func = function (i, j) return i + j - 1 end
    else
        fill_func = function (i, j) return 0 end
    end

    for i = 1, m do
        for j = 1, n do
            table.insert(mat, fill_func(i, j));
        end
    end

    return mat
end

local function print_first_elems(mat, n)
    io.write(tostring(mat[1]))
    for i = 2, math.min(10, n) do
        io.write(", ", tostring(mat[i]))
    end
    print()
end

local function time_func(func)
    local ti = os.clock()
    func()
    local tf = os.clock()
    return (tf - ti)  -- seconds
end

local function on_mult(m, n, p)
    local temp

    local mat_a = init_array(m, n, true)
    local mat_b = init_array(n, p, true)
    local mat_c = init_array(m, p, false)

    execute_mult = function()
        for i = 0, m - 1 do
            for j = 0, p - 1 do
                temp = 0
                for k = 0, n - 1 do
                    temp = temp + mat_a[i * n + k + 1] * mat_b[k * p + j + 1];
                end
                mat_c[i * p + j + 1] = temp
            end
        end
    end

    time = time_func(execute_mult)

    print("Result matrix:")
    print_first_elems(mat_c, m * n);

    return time
end

local function on_mult_line(m, n, p)
    local mat_a = init_array(m, n, true)
    local mat_b = init_array(n, p, true)
    local mat_c = init_array(m, p, false)

    execute_mult = function ()
        for i = 0, m - 1 do
            for k = 0, n - 1 do
                for j = 0, p - 1 do
                    mat_c[i * p + j + 1] = mat_c[i * p + j + 1] + mat_a[i * n + k + 1] * mat_b[k * p + j + 1];
                end
            end
        end
    end

    time = time_func(execute_mult)

    print("Result matrix:")
    print_first_elems(mat_c, m * n);

    return time
end

local function print_usage()
    print("Usage: luajit matrixproduct.lua <op> <size> <output>")
    print("  <op>     : Operation mode: 1, 2 (required)")
    print("  <size>   : Size of matrix (required)")
    print("  <output> : Output filename (required)")
end

local function create_file(filename)
    local file = io.open(filename, "r")
    if file ~= nil then
        file:close()
        file = io.open(filename, "a")
        if file == nil then
            error("Failed to open file")
            return nil
        end
    else
        file = io.open(filename, "w")
        if file == nil then
            error("Failed to create file")
            return nil
        end
        file:write("OPERATION_MODE,SIZE,BLOCK_SIZE,TIME,MFLOPS\n")
    end

    return file
end

local function is_integer(num)
    return num == math.floor(num)
end

local function main()
    if #arg ~= 3 then
        print_usage()
        return
    end

    local op = tonumber(arg[1]);
    local size = tonumber(arg[2]);
    if op == nil or size == nil or not is_integer(op) or not is_integer(size) then
        print_usage()
        return
    end

    local filename = arg[3]
    local file = create_file(filename)

    if file == nil then
        return
    end

    local time
    if op == 1 then
        time = on_mult(size, size, size)
    elseif op == 2 then
        time = on_mult_line(size, size, size)
    else
        print_usage()
        return
    end

    print("Time:", time, "seconds")
    local mflops = (2 * (size ^ 3) / time) / 1e6
    file:write(table.concat({op, size, 0.0, time, mflops}, ",") .. "\n")
end

main()
