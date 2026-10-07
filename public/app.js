// ============================================================================
// FoodRush POS - Client Application Logic
// Features: Multi-Table Dine-In, Live Kitchen Board, Past Bills & Thermal Receipts,
// Staff Attendance, 6x7 Sales Matrix, Viva Examiner Mode
// ============================================================================

const appState = {
    activeView: 'billing',
    activeTableId: 1,
    selectedOutletId: 0,
    searchQuery: '',
    outlets: [],
    tables: [],
    dishes: [],
    staff: [],
    activeOrders: [],
    pastBills: [],
    salesMatrixData: null,
    currentOrder: null,
    vivaMode: false,
    lastModules: []
};

// API Helper
async function apiCall(endpoint, method = 'GET', body = null) {
    try {
        const options = {
            method,
            headers: { 'Content-Type': 'application/json' }
        };
        if (body) options.body = JSON.stringify(body);

        const res = await fetch(endpoint, options);
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const data = await res.json();

        // Track viva module metadata
        if (data.modules && data.modules.length > 0) {
            appState.lastModules = data.modules;
            updateVivaBanner(data.modules[0]);
        }
        return data;
    } catch (err) {
        console.error(`API Error on ${endpoint}:`, err);
        return { success: false, message: err.message, data: null };
    }
}

// ============================================================================
// INITIALIZATION
// ============================================================================
document.addEventListener('DOMContentLoaded', async () => {
    initKeyboardShortcuts();
    await loadInitialState();
    handleHashNavigation();
});

async function loadInitialState() {
    const res = await apiCall('/api/initial-state');
    if (res && res.data) {
        appState.outlets = res.data.outlets || [];
        appState.tables = res.data.tables || [];
        appState.staff = res.data.staff || [];
        appState.activeTableId = res.data.activeTableId || 1;
        appState.currentOrder = res.data.activeCart || null;
    }

    // Load full dish catalog
    const menuRes = await apiCall('/api/menu?outletId=0');
    if (menuRes && menuRes.data) {
        appState.dishes = menuRes.data;
    }

    // Load active kitchen tickets
    await refreshActiveOrders();

    // Render initial views
    renderTablesGrid();
    renderDishesGrid();
    renderOrderPanel();
    updateHeaderPill();
}

// Navigation Handler
function navigateTo(viewName) {
    appState.activeView = viewName;
    window.location.hash = viewName;

    // Update nav links
    document.querySelectorAll('.header-nav .nav-link').forEach(btn => {
        btn.classList.remove('active');
    });
    const activeNav = document.getElementById(`nav${capitalize(viewName)}`);
    if (activeNav) activeNav.classList.add('active');

    // Switch views
    document.querySelectorAll('.app-view').forEach(view => {
        view.classList.remove('active');
    });
    const targetView = document.getElementById(`view${capitalize(viewName)}`);
    if (targetView) targetView.classList.add('active');

    // Load view data
    if (viewName === 'kitchen') refreshActiveOrders();
    else if (viewName === 'bills') loadPastBills();
    else if (viewName === 'staff') loadStaff();
    else if (viewName === 'sales') loadSalesAnalytics();
}

function handleHashNavigation() {
    const hash = window.location.hash.replace('#', '');
    if (hash && ['billing', 'kitchen', 'bills', 'staff', 'sales'].includes(hash)) {
        navigateTo(hash);
    } else {
        navigateTo('billing');
    }
}

window.addEventListener('hashchange', handleHashNavigation);

function capitalize(s) {
    return s.charAt(0).toUpperCase() + s.slice(1);
}

// ============================================================================
// TABLES & BILLING VIEW
// ============================================================================

function renderTablesGrid() {
    const container = document.getElementById('tablesGrid');
    if (!container) return;

    container.innerHTML = appState.tables.map(tbl => {
        const isSelected = tbl.id === appState.activeTableId;
        const isOccupied = tbl.status === 'OCCUPIED';
        const statusClass = isOccupied ? 'status-occupied' : 'status-vacant';

        return `
            <div class="table-card ${isSelected ? 'selected' : ''} ${isOccupied ? 'occupied' : ''}" onclick="selectTable(${tbl.id})">
                <div class="table-card-top">
                    <span class="table-num">T${tbl.id}</span>
                    <span class="table-status-pill ${statusClass}">${tbl.status}</span>
                </div>
                <div class="table-name">${tbl.name}</div>
                <div class="table-meta">${tbl.capacity} Seats · ${tbl.serverName}</div>
            </div>
        `;
    }).join('');
}

async function selectTable(tableId) {
    appState.activeTableId = tableId;
    renderTablesGrid();
    updateHeaderPill();

    const res = await apiCall('/api/tables/select', 'POST', { tableId });
    if (res && res.data) {
        appState.currentOrder = res.data;
        renderOrderPanel();
    }
}

function updateHeaderPill() {
    const tbl = appState.tables.find(t => t.id === appState.activeTableId);
    const pill = document.getElementById('headerActiveTable');
    if (pill) {
        pill.textContent = tbl ? tbl.name : `Table ${appState.activeTableId}`;
    }
}

// OUTLET TABS FILTER (FIX FOR PRODUCTS NOT SHOWING)
function selectOutlet(outletId) {
    appState.selectedOutletId = outletId;

    // Update tab visual states
    const tabs = document.querySelectorAll('.outlet-tab');
    tabs.forEach((tab, index) => {
        if (index === outletId) tab.classList.add('active');
        else tab.classList.remove('active');
    });

    // Clear search filter when selecting specific outlet tab for clean list
    if (appState.searchQuery) {
        appState.searchQuery = '';
        const searchInput = document.getElementById('dishSearchInput');
        if (searchInput) searchInput.value = '';
        const clearBtn = document.getElementById('searchClearBtn');
        if (clearBtn) clearBtn.style.display = 'none';
        const dym = document.getElementById('didYouMeanBar');
        if (dym) dym.style.display = 'none';
    }

    renderDishesGrid();
}

function renderDishesGrid() {
    const container = document.getElementById('dishesGrid');
    if (!container) return;

    let list = appState.dishes;

    // Filter by outlet if selected
    if (appState.selectedOutletId > 0) {
        list = list.filter(d => (d.outletId === appState.selectedOutletId || d.restaurantId === appState.selectedOutletId));
    }

    // Filter by search query if non-empty
    if (appState.searchQuery.trim().length > 0) {
        const q = appState.searchQuery.toLowerCase();
        list = list.filter(d => 
            d.name.toLowerCase().includes(q) || 
            d.category.toLowerCase().includes(q)
        );
    }

    if (list.length === 0) {
        container.innerHTML = `
            <div class="empty-dishes-box">
                <p>No dishes found matching your current filter.</p>
                <button class="btn-secondary" onclick="selectOutlet(0)">View All Dishes</button>
            </div>
        `;
        return;
    }

    container.innerHTML = list.map(dish => {
        const isVegBadge = dish.isVeg 
            ? `<span class="dietary-badge veg">VEG</span>` 
            : `<span class="dietary-badge non-veg">NON-VEG</span>`;
        const specialBadge = dish.isChefSpecial ? `<span class="dietary-badge special">CHEF'S SPECIAL</span>` : '';

        return `
            <div class="dish-card">
                <div class="dish-card-header">
                    <div>
                        <div class="dish-title">${dish.name}</div>
                        <div class="dish-category">${dish.category} · ★ ${dish.rating.toFixed(1)} (${dish.ratingCount})</div>
                    </div>
                    <div class="dish-badge-row">
                        ${isVegBadge}
                        ${specialBadge}
                    </div>
                </div>
                <div class="dish-card-footer">
                    <div class="dish-price">₹${dish.price.toFixed(2)}</div>
                    <button class="add-dish-btn" onclick="addDishToOrder(${dish.id})">+ Add</button>
                </div>
            </div>
        `;
    }).join('');
}

// SEARCH BAR LOGIC WITH LEVENSHTEIN SUGGESTIONS
let searchDebounceTimer = null;
function handleSearch(event) {
    const val = event.target.value;
    appState.searchQuery = val;

    const clearBtn = document.getElementById('searchClearBtn');
    if (clearBtn) clearBtn.style.display = val ? 'block' : 'none';

    clearTimeout(searchDebounceTimer);
    searchDebounceTimer = setTimeout(async () => {
        if (val.trim().length >= 2) {
            const res = await apiCall(`/api/search?q=${encodeURIComponent(val)}`);
            if (res && res.data && res.data.didYouMean) {
                showDidYouMean(res.data.didYouMean);
            } else {
                hideDidYouMean();
            }
        } else {
            hideDidYouMean();
        }
        renderDishesGrid();
    }, 150);
}

function clearSearch() {
    appState.searchQuery = '';
    const input = document.getElementById('dishSearchInput');
    if (input) input.value = '';
    const clearBtn = document.getElementById('searchClearBtn');
    if (clearBtn) clearBtn.style.display = 'none';
    hideDidYouMean();
    renderDishesGrid();
}

function showDidYouMean(term) {
    const bar = document.getElementById('didYouMeanBar');
    const chip = document.getElementById('suggestionChip');
    if (bar && chip) {
        chip.textContent = term;
        bar.style.display = 'flex';
    }
}

function hideDidYouMean() {
    const bar = document.getElementById('didYouMeanBar');
    if (bar) bar.style.display = 'none';
}

function applySuggestion() {
    const chip = document.getElementById('suggestionChip');
    if (chip) {
        const text = chip.textContent;
        const input = document.getElementById('dishSearchInput');
        if (input) input.value = text;
        appState.searchQuery = text;
        hideDidYouMean();
        renderDishesGrid();
    }
}

// ============================================================================
// ORDER ACTIONS (ARRAYSTACK LIFO UNDO)
// ============================================================================

async function addDishToOrder(itemId) {
    const res = await apiCall('/api/cart/add', 'POST', {
        itemId,
        quantity: 1,
        tableId: appState.activeTableId
    });
    if (res && res.success) {
        appState.currentOrder = res.data;
        renderOrderPanel();
    }
}

async function removeDishFromOrder(itemId) {
    const res = await apiCall('/api/cart/remove', 'POST', {
        itemId,
        tableId: appState.activeTableId
    });
    if (res && res.success) {
        appState.currentOrder = res.data;
        renderOrderPanel();
    }
}

async function undoOrderAction() {
    const res = await apiCall('/api/cart/undo', 'POST', {
        tableId: appState.activeTableId
    });
    if (res && res.success) {
        appState.currentOrder = res.data;
        renderOrderPanel();
    }
}

async function clearDraftOrder() {
    const res = await apiCall('/api/cart/clear', 'POST', {
        tableId: appState.activeTableId
    });
    if (res && res.success) {
        appState.currentOrder = res.data;
        renderOrderPanel();
    }
}

async function applyCoupon() {
    const input = document.getElementById('orderCouponInput');
    const code = input ? input.value.trim().toUpperCase() : '';
    if (!code) return;

    const res = await apiCall('/api/coupon', 'POST', {
        code,
        tableId: appState.activeTableId
    });

    const statusEl = document.getElementById('couponStatusText');
    if (statusEl) {
        if (res && res.success) {
            statusEl.textContent = `Applied: ${code} (${res.data.discountPercent}% off)`;
            statusEl.className = 'coupon-status-text success';
            appState.currentOrder = res.data;
            renderOrderPanel();
        } else {
            statusEl.textContent = 'Invalid coupon code';
            statusEl.className = 'coupon-status-text error';
        }
    }
}

function renderOrderPanel() {
    const tbl = appState.tables.find(t => t.id === appState.activeTableId);
    const titleEl = document.getElementById('orderPanelTableName');
    const subEl = document.getElementById('orderPanelTableSub');
    if (titleEl && tbl) titleEl.textContent = tbl.name;
    if (subEl && tbl) subEl.textContent = `${tbl.section} · Server: ${tbl.serverName}`;

    const order = appState.currentOrder;
    const itemsContainer = document.getElementById('orderItemsList');

    if (!order || !order.items || order.items.length === 0) {
        if (itemsContainer) {
            itemsContainer.innerHTML = `
                <div class="empty-order-state">
                    <p>No dishes added for this table yet.</p>
                    <p class="empty-tip">Select dishes from the catalog on the left to punch an order.</p>
                </div>
            `;
        }
        updateFinancialSummary(0, 0, 0, 0, 0);
        return;
    }

    // Render line items
    itemsContainer.innerHTML = order.items.map(item => `
        <div class="order-line-item">
            <div class="line-item-left">
                <span class="line-item-name">${item.name}</span>
                <span class="line-item-sub">₹${item.price.toFixed(2)} each</span>
            </div>
            <div class="line-item-right">
                <div class="qty-pill">
                    <button class="qty-btn" onclick="changeQty(${item.itemId}, -1)">−</button>
                    <span class="qty-num">${item.quantity}</span>
                    <button class="qty-btn" onclick="changeQty(${item.itemId}, 1)">+</button>
                </div>
                <span class="line-item-total">₹${(item.price * item.quantity).toFixed(2)}</span>
                <button class="line-remove-btn" onclick="removeDishFromOrder(${item.itemId})">✕</button>
            </div>
        </div>
    `).join('');

    updateFinancialSummary(
        order.subtotal || 0,
        order.discountAmount || 0,
        order.gstTax || 0,
        order.serviceCharge || 0,
        order.netTotal || 0
    );
}

async function changeQty(itemId, delta) {
    if (delta > 0) {
        await addDishToOrder(itemId);
    } else {
        // Decrease quantity or remove
        const item = appState.currentOrder?.items?.find(i => i.itemId === itemId);
        if (item && item.quantity <= 1) {
            await removeDishFromOrder(itemId);
        } else {
            // Revert by adding -1 or using remove
            await apiCall('/api/cart/add', 'POST', {
                itemId,
                quantity: -1,
                tableId: appState.activeTableId
            });
            const res = await apiCall(`/api/cart?tableId=${appState.activeTableId}`);
            if (res && res.data) {
                appState.currentOrder = res.data;
                renderOrderPanel();
            }
        }
    }
}

function updateFinancialSummary(subtotal, discount, gst, serviceCharge, total) {
    document.getElementById('summarySubtotal').textContent = `₹${subtotal.toFixed(2)}`;
    const discRow = document.getElementById('summaryDiscountRow');
    if (discRow) {
        discRow.style.display = discount > 0 ? 'flex' : 'none';
        document.getElementById('summaryDiscount').textContent = `-₹${discount.toFixed(2)}`;
    }
    document.getElementById('summaryGst').textContent = `₹${gst.toFixed(2)}`;
    document.getElementById('summaryServiceCharge').textContent = `₹${serviceCharge.toFixed(2)}`;
    document.getElementById('summaryTotal').textContent = `₹${total.toFixed(2)}`;
}

// SUBMIT KOT (DISPATCH TO KITCHEN QUEUE)
async function submitKOTOrder() {
    const guestInput = document.getElementById('orderGuestInput');
    const guestName = guestInput ? guestInput.value.trim() : 'Guest';
    const expressCheck = document.getElementById('orderExpressCheckbox');
    const isExpress = expressCheck ? expressCheck.checked : false;

    const res = await apiCall('/api/kot/submit', 'POST', {
        guestName,
        isExpress,
        tableId: appState.activeTableId
    });

    if (res && res.success) {
        alert(`KOT Dispatched: Order #${res.data.orderId} sent to Kitchen Circular Queue!`);
        // Refresh tables and orders
        await loadInitialState();
    } else {
        alert(res?.message || 'Failed to submit KOT');
    }
}

// SETTLE & PRINT BILL
async function settleAndGenerateBill() {
    const res = await apiCall('/api/bill/generate', 'POST', {
        tableId: appState.activeTableId,
        paymentMethod: 'UPI'
    });

    if (res && res.success) {
        showReceiptModal(res.data.asciiPrintText);
        await loadInitialState();
    } else {
        alert(res?.message || 'Could not settle bill for this table (ensure table has an active order)');
    }
}

// ============================================================================
// LIVE KITCHEN KOT BOARD (CONCURRENT TICKETS)
// ============================================================================

async function refreshActiveOrders() {
    const res = await apiCall('/api/orders/active');
    if (res && res.data) {
        appState.activeOrders = res.data;
        renderKitchenBoard();
        const badge = document.getElementById('headerKotCount');
        if (badge) badge.textContent = appState.activeOrders.length;
    }
}

function renderKitchenBoard() {
    const container = document.getElementById('kotBoardGrid');
    if (!container) return;

    if (appState.activeOrders.length === 0) {
        container.innerHTML = `
            <div class="empty-state-card">
                <p>All kitchen orders have been prepared and served.</p>
                <p class="empty-tip">Punch a new KOT from Tables & Billing to see it appear here.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = appState.activeOrders.map(kot => {
        const isExpress = kot.isExpress ? `<span class="kot-express-badge">EXPRESS VIP</span>` : '';
        const stage = kot.status;

        return `
            <div class="kot-card ${kot.isExpress ? 'express' : ''}">
                <div class="kot-header">
                    <div>
                        <div class="kot-title">KOT #${kot.orderId} · ${kot.tableName}</div>
                        <div class="kot-meta">${kot.section} · Server: ${kot.serverName} · ${kot.orderTime}</div>
                    </div>
                    ${isExpress}
                </div>

                <div class="kot-items-list">
                    ${kot.items.map(it => `
                        <div class="kot-item-line">
                            <span class="kot-item-qty">${it.quantity}x</span>
                            <span class="kot-item-name">${it.name}</span>
                        </div>
                    `).join('')}
                </div>

                <div class="kot-footer">
                    <span class="kot-stage-pill stage-${stage.toLowerCase()}">${stage}</span>
                    <div class="kot-actions">
                        ${stage === 'ORDERED' ? `<button class="btn-stage" onclick="advanceKotStage(${kot.orderId}, 'PREPARING')">Start Prep →</button>` : ''}
                        ${stage === 'PREPARING' ? `<button class="btn-stage success" onclick="advanceKotStage(${kot.orderId}, 'SERVED')">Mark Served ✓</button>` : ''}
                        ${stage === 'SERVED' ? `<span class="served-text">Ready for billing</span>` : ''}
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

async function advanceKotStage(orderId, stage) {
    const res = await apiCall('/api/orders/stage', 'POST', { orderId, stage });
    if (res && res.success) {
        await refreshActiveOrders();
    }
}

// ============================================================================
// PAST BILLS & RECEIPTS ARCHIVE
// ============================================================================

async function loadPastBills() {
    const res = await apiCall('/api/bills');
    if (res && res.data) {
        appState.pastBills = res.data;
        renderPastBills();
    }
}

function renderPastBills() {
    const tbody = document.getElementById('pastBillsTableBody');
    if (!tbody) return;

    if (appState.pastBills.length === 0) {
        tbody.innerHTML = `<tr><td colspan="9" style="text-align:center; padding: 32px;">No settled bills archived yet.</td></tr>`;
        return;
    }

    tbody.innerHTML = appState.pastBills.map(b => `
        <tr>
            <td><strong>#${b.billId}</strong></td>
            <td><code class="invoice-code">${b.invoiceCode}</code></td>
            <td>${b.tableName}</td>
            <td>${b.guestName}</td>
            <td>${b.serverName}</td>
            <td>${b.itemCount} items</td>
            <td><span class="payment-pill">${b.paymentMethod}</span></td>
            <td><strong>₹${b.netTotal.toFixed(2)}</strong></td>
            <td>
                <button class="btn-print-sm" onclick="printBillReceipt(${b.billId})">Print Receipt</button>
            </td>
        </tr>
    `).join('');
}

async function printBillReceipt(billId) {
    const res = await apiCall(`/api/bill/print?billId=${billId}`);
    if (res && res.data && res.data.asciiReceipt) {
        showReceiptModal(res.data.asciiReceipt);
    }
}

// THERMAL RECEIPT MODAL
function showReceiptModal(asciiText) {
    const modal = document.getElementById('receiptModal');
    const content = document.getElementById('thermalReceiptContent');
    if (modal && content) {
        content.textContent = asciiText;
        modal.style.display = 'flex';
    }
}

function closeReceiptModal() {
    const modal = document.getElementById('receiptModal');
    if (modal) modal.style.display = 'none';
}

function printReceiptToWindow() {
    window.print();
}

// ============================================================================
// STAFF ATTENDANCE REGISTER
// ============================================================================

async function loadStaff() {
    const res = await apiCall('/api/staff');
    if (res && res.data) {
        appState.staff = res.data;
        renderStaffGrid();
    }
}

function renderStaffGrid() {
    const container = document.getElementById('staffGrid');
    if (!container) return;

    container.innerHTML = appState.staff.map(s => `
        <div class="staff-card ${s.isPresent ? 'present' : 'absent'}">
            <div class="staff-top">
                <div>
                    <div class="staff-name">${s.name}</div>
                    <div class="staff-role">${s.role}</div>
                </div>
                <span class="staff-status-badge ${s.isPresent ? 'on-duty' : 'off-duty'}">
                    ${s.isPresent ? 'ON DUTY' : 'OFF DUTY'}
                </span>
            </div>
            <div class="staff-meta">
                <div>Shift: <strong>${s.shift}</strong></div>
                <div>Hours Clocked: <strong>${s.hoursWorked.toFixed(1)} hrs</strong></div>
                <div>Contact: <strong>${s.phone}</strong></div>
            </div>
            <div class="staff-toggle-row">
                <button class="btn-toggle ${s.isPresent ? 'btn-danger' : 'btn-success'}" onclick="toggleStaffAttendance(${s.id}, ${!s.isPresent})">
                    ${s.isPresent ? 'Clock Out' : 'Clock In (Present)'}
                </button>
            </div>
        </div>
    `).join('');
}

async function toggleStaffAttendance(staffId, isPresent) {
    const res = await apiCall('/api/staff/attendance', 'POST', {
        staffId,
        isPresent,
        hoursWorked: isPresent ? 8.0 : 0.0
    });
    if (res && res.success) {
        await loadStaff();
    }
}

// ============================================================================
// SALES ANALYTICS & MATRIX
// ============================================================================

async function loadSalesAnalytics() {
    const res = await apiCall('/api/sales-matrix');
    if (res && res.data) {
        appState.salesMatrixData = res.data;
        renderSalesAnalytics();
    }
}

function renderSalesAnalytics() {
    const d = appState.salesMatrixData;
    if (!d) return;

    // KPIs
    document.getElementById('kpiGrandTotal').textContent = `₹${d.grandTotal.toLocaleString('en-IN')}`;
    document.getElementById('kpiPeakAmount').textContent = `₹${d.peakSalesAmount.toLocaleString('en-IN')}`;
    const busiestOutletName = appState.outlets[d.busiestOutlet]?.name || `Outlet ${d.busiestOutlet + 1}`;
    const busiestDayName = d.days[d.busiestDay] || 'Saturday';
    document.getElementById('kpiPeakSlot').textContent = `${busiestOutletName} on ${busiestDayName}`;
    document.getElementById('kpiAvgDaily').textContent = `₹${Math.round(d.grandTotal / 7).toLocaleString('en-IN')}`;

    // 2D Matrix Table
    const table = document.getElementById('salesMatrixTable');
    if (!table) return;

    let html = `
        <thead>
            <tr>
                <th>Kitchen Outlet</th>
                ${d.days.map(day => `<th>${day}</th>`).join('')}
                <th>Weekly Total</th>
            </tr>
        </thead>
        <tbody>
    `;

    for (let r = 0; r < d.sales.length; ++r) {
        const outletName = appState.outlets[r]?.name || `Outlet ${r + 1}`;
        html += `
            <tr>
                <td><strong>${outletName}</strong></td>
                ${d.sales[r].map((amt, c) => {
                    const isPeak = (r === d.busiestOutlet && c === d.busiestDay);
                    return `<td class="${isPeak ? 'peak-cell' : ''}">₹${amt.toLocaleString('en-IN')}</td>`;
                }).join('')}
                <td><strong>₹${d.outletWeeklyTotals[r].toLocaleString('en-IN')}</strong></td>
            </tr>
        `;
    }

    // Daily totals row
    html += `
        <tr class="daily-totals-row">
            <td><strong>Daily Platform Total</strong></td>
            ${d.dailyTotals.map(tot => `<td><strong>₹${tot.toLocaleString('en-IN')}</strong></td>`).join('')}
            <td><strong class="grand-total-val">₹${d.grandTotal.toLocaleString('en-IN')}</strong></td>
        </tr>
        </tbody>
    `;

    table.innerHTML = html;
}

// ============================================================================
// EXAMINER / VIVA MODE (KEY: 'V')
// ============================================================================

function initKeyboardShortcuts() {
    window.addEventListener('keydown', (e) => {
        if (e.key === 'v' || e.key === 'V') {
            if (e.target.tagName !== 'INPUT' && e.target.tagName !== 'TEXTAREA') {
                toggleVivaMode(!appState.vivaMode);
            }
        }
    });
}

function toggleVivaMode(enable) {
    appState.vivaMode = enable;
    const banner = document.getElementById('vivaTopBanner');
    if (banner) {
        banner.style.display = enable ? 'block' : 'none';
    }
}

function updateVivaBanner(moduleTag) {
    const badge = document.getElementById('vivaHandledBadge');
    if (badge) {
        badge.textContent = `Handled by: ${moduleTag}`;
    }
}

async function openVivaModal(type) {
    const modal = document.getElementById('vivaModal');
    const title = document.getElementById('vivaModalTitle');
    const body = document.getElementById('vivaModalBody');
    if (!modal || !title || !body) return;

    modal.style.display = 'flex';

    if (type === 'inspector') {
        title.textContent = 'Engine Memory & Buffer Inspector (ArrayStack & CircularQueue)';
        const res = await apiCall('/api/inspect');
        body.innerHTML = `
            <div class="viva-inspector-content">
                <p><strong>Module VIII: Custom ArrayStack</strong></p>
                <pre class="json-code">${JSON.stringify(res.data?.undoStack, null, 2)}</pre>
                <p><strong>Module IX: Custom CircularQueue</strong></p>
                <pre class="json-code">${JSON.stringify(res.data?.kitchenQueue, null, 2)}</pre>
                <p><strong>Module III: 1D Array Statistics</strong></p>
                <pre class="json-code">${JSON.stringify(res.data?.arrayStats, null, 2)}</pre>
            </div>
        `;
    } else if (type === 'benchmarks') {
        title.textContent = 'Module VII: Timed Stopwatch Empirical Benchmarks';
        const res = await apiCall('/api/benchmark');
        body.innerHTML = `
            <div class="viva-bench-content">
                <p>Empirical Stopwatch Measurements on real CPU cycles:</p>
                <pre class="json-code">${JSON.stringify(res.data, null, 2)}</pre>
            </div>
        `;
    } else if (type === 'compare') {
        title.textContent = 'Module VIII/IX/X: Custom Array Structures vs STL Live Benchmark';
        const res = await apiCall('/api/compare-ds');
        body.innerHTML = `
            <div class="viva-compare-content">
                <p>50,000 Push/Pop and Enqueue/Dequeue operations comparison:</p>
                <pre class="json-code">${JSON.stringify(res.data, null, 2)}</pre>
            </div>
        `;
    } else if (type === 'map') {
        title.textContent = 'CS Syllabus Modules I-X Implementation Mapping';
        body.innerHTML = `
            <div class="viva-map-content">
                <table class="data-table">
                    <thead>
                        <tr><th>Module</th><th>Topic</th><th>Real POS Feature</th><th>Time Complexity</th></tr>
                    </thead>
                    <tbody>
                        <tr><td>Module I</td><td>Basics & Financial Types</td><td>GST & Service Charge Invoicing</td><td>O(1)</td></tr>
                        <tr><td>Module II</td><td>Functions & Control Flow</td><td>Command Dispatcher & Overloading</td><td>O(1)</td></tr>
                        <tr><td>Module III</td><td>1D Arrays</td><td>Prices, Stock, Ratings, Staff Roster</td><td>O(N)</td></tr>
                        <tr><td>Module IV</td><td>2D Arrays</td><td>6x7 Weekly Sales Matrix & Seating</td><td>O(R * D)</td></tr>
                        <tr><td>Module V</td><td>Strings & Levenshtein</td><td>Dish Search & Check-Digit Invoices</td><td>O(L1 * L2)</td></tr>
                        <tr><td>Module VI</td><td>Structures & Nested</td><td>Table, StaffMember, OrderTicket, Bill</td><td>O(1)</td></tr>
                        <tr><td>Module VII</td><td>Empirical Benchmarks</td><td>Linear vs Binary, Bubble vs Introsort</td><td>O(log N) vs O(N)</td></tr>
                        <tr><td>Module VIII</td><td>Stack (Array-based)</td><td>LIFO Table Order Action Undo Stack</td><td>O(1)</td></tr>
                        <tr><td>Module IX</td><td>Queue (Circular)</td><td>Multi-Ticket FIFO Kitchen KOT Queue</td><td>O(1)</td></tr>
                        <tr><td>Module X</td><td>STL Containers</td><td>std::deque Bills, std::map Coupons</td><td>O(log K)</td></tr>
                    </tbody>
                </table>
            </div>
        `;
    }
}

function closeVivaModal() {
    const modal = document.getElementById('vivaModal');
    if (modal) modal.style.display = 'none';
}
