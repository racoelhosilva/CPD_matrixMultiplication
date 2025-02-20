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

local function print_first_elems(mat, m, n)
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
    return (tf - ti) * 1000  -- ms
end

local function on_mult(m, n, p)
    local temp
    
    local mat_a = init_array(m, n, true)
    local mat_b = init_array(n, p, true)
    local mat_c = init_array(m, p, false)

    for i = 0, m - 1 do
        for j = 0, p - 1 do
            temp = 0
            for k = 0, n - 1 do
                temp = temp + mat_a[i * n + k + 1] * mat_b[k * p + j + 1];
            end
            mat_c[i * p + j + 1] = temp
        end
    end

    print("Result matrix:")
    print_first_elems(mat_c, n, p);
end

local function on_mult_line(m, n, p)
    local mat_a = init_array(m, n, true)
    local mat_b = init_array(n, p, true)
    local mat_c = init_array(m, p, false)

    for i = 0, m - 1 do
        for k = 0, n - 1 do
            for j = 0, p - 1 do
                mat_c[i * p + j + 1] = mat_c[i * p + j + 1] + mat_a[i * n + k + 1] * mat_b[k * p + j + 1];
            end
        end
    end

    print("Result matrix:")
    print_first_elems(mat_c, n, p);
end

local function on_mult_block(m, n, p, blockSize)
    local mat_a = init_array(m, n, true)
    local mat_b = init_array(n, p, true)
    local mat_c = init_array(m, p, false)

    for row = 0, math.ceil(m / blockSize) - 1 do
        for col = 0, math.ceil(p / blockSize) - 1 do
            for i = row * blockSize, math.min((row + 1) * blockSize, m) - 1 do
                for k = 0, n - 1 do
                    for j = col * blockSize, math.min((col + 1) * blockSize, p) - 1 do
                        if mat_b[k * p + j + 1] == nil then
                            print(i, j, k, n * p, k * p + j + 1)
                        end
                        mat_c[i * p + j + 1] = mat_c[i * p + j + 1] + mat_a[i * n + k + 1] * mat_b[k * p + j + 1];
                    end
                end
            end
        end
    end

    print("Result matrix:")
    print_first_elems(mat_c, n, p);
end

local function print_usage()
    print("Usage: lua matrixproduct.lua <op> <lin> <col> <output> [blockSize]")
    print("  <op>       : Opration mode: 1, 2, 3 (required)")
    print("  <lin>      : Number of lines (required)")
    print("  <col>      : Number of columns (required)")
    print("  <output>   : Path to output filename (required)")
    print("  [blockSize]: Size of a block (optional)")
end

local function open_file(filename)
    local file = io.open(filename, "r")
    if file ~= nil then
        file:close()
        file = io.open(filename, "a")
        if file == nil then
            error("Failed to open file")
        end
    else
        file = io.open(filename, "w")
        if file == nil then
            error("Failed to create file")
        end
        file:write("OPERATION_MODE,LIN,COL,BLOCK_SIZE,TIME\n")
    end

    return file
end

local function main()
    if #arg < 4 or #arg > 5 then
        print_usage()
        return
    end

    local op = tonumber(arg[1]);
    if op ~= 3 and #arg > 4 then
        print_usage()
        return
    end

    local line = tonumber(arg[2])
    local col = tonumber(arg[3])
    local filename = arg[4]
    local file = open_file(filename)
    local block_size = tonumber(arg[5]);

    if line == nil or col == nil or (op == 3 and block_size == nil) then
        print_usage()
        return
    end

    local time
    if op == 1 then
        time = time_func(function () on_mult(line, col, line) end)
    elseif op == 2 then
        time = time_func(function () on_mult_line(line, col, line) end)
    elseif op == 3 then
        time = time_func(function () on_mult_block(line, col, line, block_size) end)
    else
        print_usage()
        return
    end

    print("Time:", time, "ms")
    file:write(table.concat({op, line, col, block_size, time}, ",") .. "\n")
end

main()
