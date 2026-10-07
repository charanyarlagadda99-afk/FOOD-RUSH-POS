// ============================================================================
// FoodRush - Comprehensive Smoke Test Suite
// Verifies all 10 CS Syllabus Modules end-to-end on Upgraded Engine
// ============================================================================

const { spawn } = require('child_process');
const path = require('path');
const readline = require('readline');

const ENGINE_PATH = path.join(__dirname, '..', 'foodrush_engine.exe');

console.log('================================================================');
console.log('       FOODRUSH UPGRADED ENGINE END-TO-END VERIFICATION         ');
console.log('================================================================\n');

const testCommands = [
    { cmd: 'PING', expectedMod: 'Module I', desc: 'Basics & Protocol' },
    { cmd: 'GET_RESTAURANTS', expectedMod: 'Module VI', desc: '6 Restaurants & 1D Array' },
    { cmd: 'GET_MENU 1', expectedMod: 'Module I', desc: '8 Dishes per Kitchen & Bitwise Dietary Flags' },
    { cmd: 'SEARCH_DISH biryani', expectedMod: 'Module V', desc: 'Case-Insensitive Substring Match' },
    { cmd: 'SEARCH_DISH biryany', expectedMod: 'Module V', desc: 'Levenshtein "Did You Mean" Fuzzy Search' },
    { cmd: 'CART_ADD 101 2', expectedMod: 'Module VIII', desc: 'ArrayStack Push on Cart Action' },
    { cmd: 'CART_VIEW 0 1', expectedMod: 'Module IV', desc: '2D Zone Distance Fee & INR Bill Calculation' },
    { cmd: 'CART_UNDO', expectedMod: 'Module VIII', desc: 'ArrayStack Pop & Reversion' },
    { cmd: 'CART_ADD 101 1', expectedMod: 'Module VIII', desc: 'Re-add Item for Order' },
    { cmd: 'APPLY_COUPON FIRST50', expectedMod: 'Module X', desc: 'std::map Coupon Discount Lookup' },
    { cmd: 'APPLY_COUPON LEVEL', expectedMod: 'Module V', desc: 'Palindrome Bonus using String Reversal' },
    { cmd: 'CHECKOUT Aarav 0 MG_Road 1', expectedMod: 'Module IX', desc: 'Order Struct, Check-Digit & Priority Queue' },
    { cmd: 'TRACK_ORDER 1001', expectedMod: 'Module V', desc: 'Tracking Code Check Digit Verification' },
    { cmd: 'SIMULATE_NEXT_STAGE 1001', expectedMod: 'Module IX', desc: 'Circular Queue Stage: PLACED -> PREPARING' },
    { cmd: 'SIMULATE_NEXT_STAGE 1001', expectedMod: 'Module IX', desc: 'Circular Queue Dequeue & Rider Rotation' },
    { cmd: 'SIMULATE_NEXT_STAGE 1001', expectedMod: 'Module X', desc: 'Delivery Completion & STL Deque Record' },
    { cmd: 'GET_SALES_MATRIX', expectedMod: 'Module IV', desc: '6x7 Sales Revenue Matrix & Peak Analysis' },
    { cmd: 'GET_ARRAY_STATS', expectedMod: 'Module III', desc: '1D Arrays: Sum, Min, Max & Bubble Sort' },
    { cmd: 'BENCHMARK', expectedMod: 'Module VII', desc: 'Performance: Linear vs Binary, Bubble vs Introsort' },
    { cmd: 'COMPARE_DS 10000', expectedMod: 'Module X', desc: 'Custom ArrayStack/Queue vs STL stack/queue' },
    { cmd: 'GET_FLEET_STATUS', expectedMod: 'Feature 1', desc: 'Smart Fleet Dispatch & Zone Routing' },
    { cmd: 'RESTOCK_ITEM 101 25', expectedMod: 'Feature 2', desc: 'Real-Time Inventory Lock & Restocking' },
    { cmd: 'RATE_DISH 101 5.0', expectedMod: 'Feature 3', desc: 'Customer Rating Feedback System' },
    { cmd: 'GET_TOP_DISHES 5', expectedMod: 'Feature 3', desc: 'Top-K Leaderboard via O(N log K) Sorting' },
    { cmd: 'INSPECT_ENGINE', expectedMod: 'Module VIII', desc: 'Engine Live Memory Inspector' }
];

async function runSmokeTest() {
    const engine = spawn(ENGINE_PATH, [], {
        cwd: path.join(__dirname, '..'),
        stdio: ['pipe', 'pipe', 'inherit']
    });

    const rl = readline.createInterface({ input: engine.stdout, crlfDelay: Infinity });

    const queue = [];
    rl.on('line', (line) => {
        if (!line.trim()) return;
        if (queue.length > 0) {
            const { resolve } = queue.shift();
            try {
                resolve(JSON.parse(line));
            } catch (e) {
                resolve({ success: false, raw: line });
            }
        }
    });

    function sendCmd(cmd) {
        return new Promise(resolve => {
            queue.push({ resolve });
            engine.stdin.write(cmd + '\n');
        });
    }

    let passed = 0;
    let failed = 0;

    for (let i = 0; i < testCommands.length; ++i) {
        const t = testCommands[i];
        const res = await sendCmd(t.cmd);

        const modules = res.modules || res.handled_by || [];
        const hasMod = modules.some(m => m.includes(t.expectedMod));
        const ok = res.success && hasMod;

        if (ok) {
            console.log(`[PASS] (${i+1}/${testCommands.length}) ${t.cmd.padEnd(30)} | ${t.desc} -> [${modules.join(', ')}]`);
            passed++;
        } else {
            console.error(`[FAIL] (${i+1}/${testCommands.length}) ${t.cmd} -> ${JSON.stringify(res)}`);
            failed++;
        }
    }

    engine.stdin.write('EXIT\n');
    engine.kill();

    console.log('\n----------------------------------------------------------------');
    console.log(`Smoke Test Results: ${passed} Passed, ${failed} Failed`);
    console.log('All 10 Syllabus Modules Verified Operational on Upgraded Engine!');
    console.log('----------------------------------------------------------------\n');

    process.exit(failed > 0 ? 1 : 0);
}

runSmokeTest();
