// ============================================================================
// FoodRush / RestoRush - Node.js Persistent Bridge Server
// Connects the Web UI to the persistent C++ Engine via stdin/stdout line protocol
// Supports: Dine-In Tables, Multi-Order Kitchen Board, Bills & Printing, Staff Attendance
// ============================================================================

const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');
const readline = require('readline');

const DEFAULT_PORT = process.env.PORT ? parseInt(process.env.PORT) : 3000;
let currentPort = DEFAULT_PORT;
const ENGINE_PATH = path.join(__dirname, '..', 'foodrush_engine.exe');
const PUBLIC_DIR = path.join(__dirname, '..', 'public');

console.log('============================================================');
console.log('FoodRush POS Persistent Bridge Server');
console.log(`Target C++ Engine Binary: ${ENGINE_PATH}`);
console.log('============================================================');

if (!fs.existsSync(ENGINE_PATH)) {
    console.error(`ERROR: Engine executable not found at ${ENGINE_PATH}`);
    console.error('Please compile the C++ engine first with: g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe');
    process.exit(1);
}

// Spawn the C++ engine ONCE as a persistent child process
const engineProcess = spawn(ENGINE_PATH, [], {
    cwd: path.join(__dirname, '..'),
    stdio: ['pipe', 'pipe', 'inherit']
});

engineProcess.on('error', (err) => {
    console.error('Failed to spawn C++ Engine process:', err);
});

engineProcess.on('exit', (code, signal) => {
    console.error(`C++ Engine exited with code ${code}, signal ${signal}`);
});

const rl = readline.createInterface({
    input: engineProcess.stdout,
    crlfDelay: Infinity
});

const pendingQueue = [];

rl.on('line', (line) => {
    const trimmed = line.trim();
    if (!trimmed) return;

    if (pendingQueue.length > 0) {
        const { resolve } = pendingQueue.shift();
        try {
            const parsed = JSON.parse(trimmed);
            resolve(parsed);
        } catch (e) {
            resolve({
                success: false,
                message: 'Failed to parse engine JSON response',
                raw: trimmed,
                handled_by: ['Server Line Protocol']
            });
        }
    }
});

function sendEngineCommand(commandString) {
    return new Promise((resolve, reject) => {
        pendingQueue.push({ resolve, reject });
        engineProcess.stdin.write(commandString.trim() + '\n');
    });
}

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon',
    '.png': 'image/png',
    '.woff2': 'font/woff2'
};

function readJsonBody(req) {
    return new Promise((resolve) => {
        let body = '';
        req.on('data', chunk => body += chunk);
        req.on('end', () => {
            try {
                resolve(body ? JSON.parse(body) : {});
            } catch (e) {
                resolve({});
            }
        });
    });
}

const server = http.createServer(async (req, res) => {
    const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    const pathname = parsedUrl.pathname;

    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    // ------------------------------------------------------------------------
    // REST API ROUTING
    // ------------------------------------------------------------------------
    if (pathname.startsWith('/api/')) {
        res.setHeader('Content-Type', 'application/json; charset=utf-8');

        try {
            // Initial State & Overview
            if ((pathname === '/api/initial-state' || pathname === '/api/ping') && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_INITIAL_STATE');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Kitchen Outlets / Restaurants
            if ((pathname === '/api/restaurants' || pathname === '/api/outlets') && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_OUTLETS');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Menu Items (supports ?outletId= or ?restaurantId=)
            if (pathname === '/api/menu' && req.method === 'GET') {
                const outletId = parsedUrl.searchParams.get('restaurantId') || parsedUrl.searchParams.get('outletId') || '0';
                const resp = await sendEngineCommand(`GET_MENU ${outletId}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Dining Tables Status
            if (pathname === '/api/tables' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_TABLES');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Switch Active Table
            if (pathname === '/api/tables/select' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`SELECT_TABLE ${data.tableId || 1}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // View Draft Table Order / Cart
            if ((pathname === '/api/cart' || pathname === '/api/order/current') && req.method === 'GET') {
                const tableId = parsedUrl.searchParams.get('tableId') || '1';
                const resp = await sendEngineCommand(`ORDER_VIEW ${tableId}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Add Dish to Table Order
            if ((pathname === '/api/cart/add' || pathname === '/api/order/add') && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`ORDER_ADD ${data.itemId} ${data.quantity || 1} ${data.tableId || 1}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Remove Dish from Table Order
            if ((pathname === '/api/cart/remove' || pathname === '/api/order/remove') && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`ORDER_REMOVE ${data.itemId} ${data.tableId || 1}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Undo Last Table Order Action (LIFO Stack)
            if ((pathname === '/api/cart/undo' || pathname === '/api/order/undo') && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`ORDER_UNDO ${data.tableId || 1}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Clear Draft Table Order
            if ((pathname === '/api/cart/clear' || pathname === '/api/order/clear') && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`ORDER_CLEAR ${data.tableId || 1}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Apply Coupon Discount
            if (pathname === '/api/coupon' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`APPLY_COUPON ${data.code || ''} ${data.tableId || 1}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Dispatch KOT to Kitchen Queue
            if ((pathname === '/api/kot/submit' || pathname === '/api/checkout') && req.method === 'POST') {
                const data = await readJsonBody(req);
                const guest = (data.guestName || data.customerName || 'Guest').replace(/\s+/g, '_');
                const isExpress = data.isExpress ? '1' : '0';
                const tableId = data.tableId || 1;
                const resp = await sendEngineCommand(`SUBMIT_KOT ${guest} ${isExpress} ${tableId}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Live Kitchen KOT Orders Queue
            if ((pathname === '/api/orders/active' || pathname === '/api/orders' || pathname === '/api/kot') && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_ACTIVE_ORDERS');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Advance Order Stage (ORDERED -> PREPARING -> SERVED)
            if (pathname === '/api/orders/stage' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`UPDATE_ORDER_STAGE ${data.orderId} ${data.stage || 'PREPARING'}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Generate & Settle Bill
            if (pathname === '/api/bill/generate' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const payment = data.paymentMethod || 'UPI';
                const coupon = data.coupon || '';
                const resp = await sendEngineCommand(`GENERATE_BILL ${data.tableId || 1} ${payment} ${coupon}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Past Bills Archive
            if (pathname === '/api/bills' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_BILLS');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Print Thermal Receipt ASCII
            if (pathname === '/api/bill/print' && req.method === 'GET') {
                const billId = parsedUrl.searchParams.get('billId') || '5001';
                const resp = await sendEngineCommand(`PRINT_BILL ${billId}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Staff Attendance Register & Shifts
            if (pathname === '/api/staff' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_STAFF');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Mark Staff Attendance
            if (pathname === '/api/staff/attendance' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const present = data.isPresent ? '1' : '0';
                const hours = data.hoursWorked || '8';
                const resp = await sendEngineCommand(`MARK_ATTENDANCE ${data.staffId} ${present} ${hours}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Dish Search (Levenshtein did-you-mean, tokenizing, char frequency)
            if (pathname === '/api/search' && req.method === 'GET') {
                const query = parsedUrl.searchParams.get('q') || '';
                const resp = await sendEngineCommand(`SEARCH_DISHES ${query}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // 6 Outlets x 7 Days Weekly Sales Matrix
            if (pathname === '/api/sales-matrix' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_SALES_MATRIX');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Top-K Ranked Dishes
            if (pathname === '/api/top-dishes' && req.method === 'GET') {
                const k = parsedUrl.searchParams.get('k') || '5';
                const resp = await sendEngineCommand(`GET_TOP_DISHES ${k}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Rate Dish
            if (pathname === '/api/rate-dish' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`RATE_DISH ${data.dishId || data.itemId} ${data.stars || data.rating}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Restock Dish Inventory
            if (pathname === '/api/restock' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`RESTOCK_ITEM ${data.itemId} ${data.quantity}`);
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Stopwatch Performance Benchmark
            if (pathname === '/api/benchmark' && req.method === 'GET') {
                const resp = await sendEngineCommand('BENCHMARK');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Custom ArrayStack/Queue vs STL Benchmark
            if (pathname === '/api/compare-ds' && req.method === 'GET') {
                const resp = await sendEngineCommand('COMPARE_DS');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            // Memory Inspector
            if (pathname === '/api/inspect' && req.method === 'GET') {
                const resp = await sendEngineCommand('INSPECT_ENGINE');
                res.writeHead(200);
                return res.end(JSON.stringify(resp));
            }

            res.writeHead(404);
            res.end(JSON.stringify({ success: false, message: 'Unknown API endpoint: ' + pathname }));
            return;
        } catch (err) {
            res.writeHead(500);
            res.end(JSON.stringify({ success: false, message: err.message }));
            return;
        }
    }

    // ------------------------------------------------------------------------
    // STATIC FILE SERVING
    // ------------------------------------------------------------------------
    let targetFile = pathname === '/' || pathname === '/admin' ? 'index.html' : pathname;
    let filePath = path.join(PUBLIC_DIR, targetFile);

    if (!filePath.startsWith(PUBLIC_DIR)) {
        res.writeHead(403);
        res.end('Access Denied');
        return;
    }

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            filePath = path.join(PUBLIC_DIR, 'index.html');
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, { 'Content-Type': contentType });
        const stream = fs.createReadStream(filePath);
        stream.pipe(res);
    });
});

function startServer(port) {
    server.listen(port, () => {
        console.log(`FoodRush POS Server running at http://localhost:${port}`);
        console.log(`- POS Cashier & Dining: http://localhost:${port}`);
        console.log(`- Kitchen KOT Board:   http://localhost:${port}#kitchen`);
        console.log(`- Past Bills Archive:   http://localhost:${port}#bills`);
        console.log(`- Staff & Attendance:   http://localhost:${port}#staff`);
        console.log(`- Sales Analytics:      http://localhost:${port}#sales`);
        console.log(`- Examiner Mode (key V): http://localhost:${port}#viva`);
    });
}

server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
        console.log(`Port ${currentPort} is busy, retrying on port ${currentPort + 1}...`);
        currentPort++;
        startServer(currentPort);
    } else {
        console.error('Server error:', err);
    }
});

startServer(currentPort);
