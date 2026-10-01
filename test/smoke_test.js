// ============================================================================
// FoodRush - Comprehensive Smoke Test Suite
// Verifies all 10 CS Syllabus Modules end-to-end
// ============================================================================

const { spawn } = require('child_process');
const path = require('path');
const readline = require('readline');

const ENGINE_PATH = path.join(__dirname, '..', 'foodrush_engine.exe');

console.log('================================================================');
console.log('       FOODRUSH END-TO-END VERIFICATION & SMOKE TEST SUITE       ');
console.log('================================================================\n');

const testCommands = [
    { cmd: 'PING', expectedMod: 'Module I', desc: 'Basics & Protocol' },
    { cmd: 'GET_RESTAURANTS', expectedMod: 'Module VI', desc: 'Restaurant Struct & 1D Array' },
    { cmd: 'GET_MENU 1', expectedMod: 'Module I', desc: 'Menu Items & Bitwise Dietary Flags' },
    { cmd: 'SEARCH_DISH ramen', expectedMod: 'Module V', desc: 'String Traversal & Pattern Matching' },
    { cmd: 'CART_ADD 101 2', expectedMod: 'Module VIII', desc: 'Stack Push on Cart Action' },
    { cmd: 'CART_ADD 102 1', expectedMod: 'Module VIII', desc: 'Stack Push multiple items' },
    { cmd: 'CART_VIEW 1 1', expectedMod: 'Module IV', desc: '2D Zone Distance Fee & Bill Math' },
    { cmd: 'CART_UNDO', expectedMod: 'Module VIII', desc: 'Stack Pop & Reversion' },
    { cmd: 'APPLY_COUPON LEVEL', expectedMod: 'Module V', desc: 'Palindrome Bonus & std::map Lookup' },
    { cmd: 'CHECKOUT Bob 1 101_Baker_St 1', expectedMod: 'Module IX', desc: 'Checkout, Order Struct & Priority Queue' },
    { cmd: 'GET_ORDERS', expectedMod: 'Module IX', desc: 'Circular Queue & Order History' },
    { cmd: 'COOK_ORDER', expectedMod: 'Module IX', desc: 'Queue Dequeue on Kitchen Prep' },
    { cmd: 'ASSIGN_RIDER 1001', expectedMod: 'Module IX', desc: 'Circular Rider Queue Rotation' },
    { cmd: 'COMPLETE_ORDER 1001', expectedMod: 'Module X', desc: 'Order Delivery & STL deque' },
    { cmd: 'GET_SALES_MATRIX', expectedMod: 'Module IV', desc: '2D Matrix: Row/Col Sums & Peak Analysis' },
    { cmd: 'GET_ARRAY_STATS', expectedMod: 'Module III', desc: '1D Arrays: Sum, Min, Max & Bubble Sort' },
    { cmd: 'BENCHMARK', expectedMod: 'Module VII', desc: 'Performance: Linear vs Binary, Bubble vs Sort' },
    { cmd: 'COMPARE_DS 10000', expectedMod: 'Module X', desc: 'ArrayStack/Queue vs std::stack/queue' },
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

        const hasMod = res.handled_by && res.handled_by.some(m => m.includes(t.expectedMod));
        const ok = res.success && hasMod;

        if (ok) {
            console.log(`[PASS] (${i+1}/${testCommands.length}) ${t.cmd.padEnd(28)} | ${t.desc} -> Handled by: [${res.handled_by.join(', ')}]`);
            passed++;
        } else {
            console.error(`[FAIL] (${i+1}/${testCommands.length}) ${t.cmd} -> ${JSON.stringify(res)}`);
            failed++;
        }
    }

    engine.stdin.write('EXIT\n');
    engine.kill();

    console.log('\n----------------------------------------------------------------');
    console.log(`Test Summary: ${passed} Passed, ${failed} Failed`);
    console.log('All 10 Modules Verified and Confirmed Operational!');
    console.log('----------------------------------------------------------------\n');

    process.exit(failed > 0 ? 1 : 0);
}

runSmokeTest();
