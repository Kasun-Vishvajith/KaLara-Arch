#include "kalara/runtime/transaction.hpp"
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include <iostream>
#include <cassert>

using namespace kalara::architecture;
using namespace kalara::runtime;
using namespace kalara::core::geometry;

Level* setupProjectWithLevel(Project& project) {
    auto* site = project.defaultSite();
    assert(site != nullptr);
    auto& bld = site->addBuilding("Main Building");
    auto& level = bld.addLevel("Ground Floor", 0.0, 3000.0, LevelType::GroundFloor);
    return &level;
}

Level* getLevel(Project& project) {
    auto* site = project.defaultSite();
    if (!site || site->buildings().empty()) return nullptr;
    auto* bld = site->defaultBuilding();
    if (!bld || bld->levels().empty()) return nullptr;
    return bld->defaultLevel();
}

void testTransactionLifecycle() {
    std::cout << "[TEST] Basic Transaction Lifecycle & Undo/Redo..." << std::endl;

    Project project("Undo Lifecycle Test");
    Level* level = setupProjectWithLevel(project);
    assert(level != nullptr);
    assert(level->walls().empty());

    TransactionManager tm;
    assert(!tm.canUndo());
    assert(!tm.canRedo());
    assert(tm.activeTransaction() == nullptr);

    // 1. Begin transaction
    bool ok = tm.beginTransaction(project, "Add perimeter walls", TransactionActor::Human);
    assert(ok);
    assert(tm.activeTransaction() != nullptr);
    assert(tm.activeTransaction()->intent == "Add perimeter walls");
    assert(tm.activeTransaction()->actor == TransactionActor::Human);

    // Cannot begin nested transaction while active
    assert(!tm.beginTransaction(project, "Nested forbidden"));

    // Add 4 walls
    auto& w1 = level->addWall(Point2D{0, 0}, Point2D{5000, 0}, 200.0);
    tm.recordOperation("Add south wall");
    auto& w2 = level->addWall(Point2D{5000, 0}, Point2D{5000, 4000}, 200.0);
    tm.recordOperation("Add east wall");
    auto& w3 = level->addWall(Point2D{5000, 4000}, Point2D{0, 4000}, 200.0);
    tm.recordOperation("Add north wall");
    auto& w4 = level->addWall(Point2D{0, 4000}, Point2D{0, 0}, 200.0);
    tm.recordOperation("Add west wall");

    assert(level->walls().size() == 4);

    // 2. Commit transaction
    ok = tm.commitTransaction(project);
    assert(ok);
    assert(tm.activeTransaction() == nullptr);
    assert(tm.canUndo());
    assert(!tm.canRedo());
    assert(tm.undoStack().size() == 1);
    assert(tm.nextUndoIntent() == "Add perimeter walls");
    assert(tm.undoStack().front().operations.size() == 4);

    // 3. Undo transaction
    ok = tm.undo(project);
    assert(ok);
    assert(!tm.canUndo());
    assert(tm.canRedo());
    assert(tm.nextRedoIntent() == "Add perimeter walls");

    // Re-fetch level after state restoration
    level = getLevel(project);
    assert(level != nullptr);
    assert(level->walls().empty()); // Restored to 0 walls

    // 4. Redo transaction
    ok = tm.redo(project);
    assert(ok);
    assert(tm.canUndo());
    assert(!tm.canRedo());

    level = getLevel(project);
    assert(level != nullptr);
    assert(level->walls().size() == 4); // Restored to 4 walls
    assert(level->walls()[0]->start == Point2D(0, 0));
    assert(level->walls()[0]->end == Point2D(5000, 0));

    std::cout << "  -> Passed basic lifecycle & undo/redo." << std::endl;
}

void testSemanticGroupedOperationsAI() {
    std::cout << "[TEST] Section 13: Grouped AI Operations Transaction..." << std::endl;

    Project project("Section 13 AI Project");
    Level* level = setupProjectWithLevel(project);
    assert(level != nullptr);

    TransactionManager tm;

    // Begin AI transaction matching Section 13 specification
    bool ok = tm.beginTransaction(project, "Create master bedroom", TransactionActor::AIAgent);
    assert(ok);

    // Op 1: Create room
    auto& room = level->addRoom("Master Bedroom", RoomType::Bedroom, {
        Point2D{0, 0}, Point2D{6000, 0}, Point2D{6000, 5000}, Point2D{0, 5000}
    });
    tm.recordOperation("1. Create room");

    // Op 2: Create four connected walls
    auto& w1 = level->addWall(Point2D{0, 0}, Point2D{6000, 0}, 200.0);
    auto& w2 = level->addWall(Point2D{6000, 0}, Point2D{6000, 5000}, 200.0);
    auto& w3 = level->addWall(Point2D{6000, 5000}, Point2D{0, 5000}, 200.0);
    auto& w4 = level->addWall(Point2D{0, 5000}, Point2D{0, 0}, 200.0);
    tm.recordOperation("2. Create four connected walls");

    // Op 3: Add door
    auto& door = level->addDoor(w1.id, 2000.0, 900.0);
    tm.recordOperation("3. Add door");

    // Op 4: Add window
    auto& window = level->addWindow(w3.id, 3000.0, 1200.0);
    tm.recordOperation("4. Add window");

    // Op 5: Add dimensions
    auto& dim = level->addDimension(Point2D{0, 5300}, Point2D{6000, 5300});
    tm.recordOperation("5. Add dimensions");

    // Op 6: Apply relevant constraints
    room.boundaryWallIds.push_back(w1.id);
    room.boundaryWallIds.push_back(w2.id);
    room.boundaryWallIds.push_back(w3.id);
    room.boundaryWallIds.push_back(w4.id);
    tm.recordOperation("6. Apply relevant constraints");

    assert(tm.activeTransaction()->operationCount() == 6);
    assert(tm.activeTransaction()->actor == TransactionActor::AIAgent);

    ok = tm.commitTransaction(project);
    assert(ok);
    assert(tm.undoStack().size() == 1);
    const auto& tx = tm.undoStack().back();
    assert(tx.intent == "Create master bedroom");
    assert(tx.actor == TransactionActor::AIAgent);
    assert(tx.operationCount() == 6);
    assert(tx.id.rfind("tx_", 0) == 0); // starts with "tx_"

    // Verify Undo reverts the entire grouped AI transaction at once
    ok = tm.undo(project);
    assert(ok);
    level = getLevel(project);
    assert(level->walls().empty());
    assert(level->doors().empty());
    assert(level->windows().empty());
    assert(level->rooms().empty());
    assert(level->dimensions().empty());

    // Verify Redo brings everything back
    ok = tm.redo(project);
    assert(ok);
    level = getLevel(project);
    assert(level->walls().size() == 4);
    assert(level->doors().size() == 1);
    assert(level->windows().size() == 1);
    assert(level->rooms().size() == 1);
    assert(level->dimensions().size() == 1);
    assert(level->rooms()[0]->name == "Master Bedroom");

    std::cout << "  -> Passed Section 13 grouped AI operations." << std::endl;
}

void testRollbackOnCancel() {
    std::cout << "[TEST] Rollback Transaction Discards Active Mutations..." << std::endl;

    Project project("Rollback Test");
    Level* level = setupProjectWithLevel(project);
    level->addWall(Point2D{0, 0}, Point2D{3000, 0}, 150.0);
    assert(level->walls().size() == 1);

    TransactionManager tm;
    tm.beginTransaction(project, "Tentative modification");
    level->addWall(Point2D{3000, 0}, Point2D{3000, 3000}, 150.0);
    assert(level->walls().size() == 2);

    // Cancel / rollback
    bool ok = tm.rollbackTransaction(project);
    assert(ok);
    assert(tm.activeTransaction() == nullptr);
    assert(!tm.canUndo());
    assert(!tm.canRedo());

    level = getLevel(project);
    assert(level->walls().size() == 1); // 2nd wall reverted

    std::cout << "  -> Passed rollback discard." << std::endl;
}

void testRedoStackInvalidation() {
    std::cout << "[TEST] Redo Stack Invalidation on New Mutation..." << std::endl;

    Project project("Redo Invalidation Test");
    Level* level = setupProjectWithLevel(project);

    TransactionManager tm;
    tm.beginTransaction(project, "Action 1");
    level->addWall(Point2D{0, 0}, Point2D{1000, 0}, 100.0);
    tm.commitTransaction(project);

    tm.beginTransaction(project, "Action 2");
    level->addWall(Point2D{1000, 0}, Point2D{2000, 0}, 100.0);
    tm.commitTransaction(project);

    assert(tm.undoStack().size() == 2);
    tm.undo(project);
    assert(tm.canRedo());
    assert(tm.redoStack().size() == 1);

    // New action performed
    level = getLevel(project);
    tm.beginTransaction(project, "Action 3 (divergent)");
    level->addWall(Point2D{1000, 0}, Point2D{1000, 2000}, 100.0);
    tm.commitTransaction(project);

    // Redo stack must be cleared
    assert(!tm.canRedo());
    assert(tm.redoStack().empty());
    assert(tm.undoStack().size() == 2); // Action 1, Action 3

    std::cout << "  -> Passed redo stack invalidation." << std::endl;
}

void testTransactionScopeRAII() {
    std::cout << "[TEST] TransactionScope RAII Guard..." << std::endl;

    Project project("RAII Scope Test");
    Level* level = setupProjectWithLevel(project);

    TransactionManager tm;

    // Test 1: Uncommitted scope auto-rolls back on exception or early return
    {
        TransactionScope scope(tm, project, "Uncommitted edit");
        level->addWall(Point2D{0, 0}, Point2D{1000, 0}, 150.0);
        assert(level->walls().size() == 1);
        // Exiting scope without commit()
    }

    level = getLevel(project);
    assert(level->walls().empty());
    assert(!tm.canUndo());

    // Test 2: Committed scope persists
    {
        TransactionScope scope(tm, project, "Committed edit");
        level->addWall(Point2D{0, 0}, Point2D{2500, 0}, 150.0);
        scope.record("Added 2500mm wall");
        bool committed = scope.commit();
        assert(committed);
    }

    level = getLevel(project);
    assert(level->walls().size() == 1);
    assert(tm.canUndo());
    assert(tm.nextUndoIntent() == "Committed edit");

    std::cout << "  -> Passed RAII TransactionScope." << std::endl;
}

void testMaxHistoryLimit() {
    std::cout << "[TEST] Max History Truncation..." << std::endl;

    Project project("History Limit Test");
    Level* level = setupProjectWithLevel(project);

    TransactionManager tm(3); // Cap at 3 transactions

    for (int i = 1; i <= 5; ++i) {
        tm.beginTransaction(project, "Action " + std::to_string(i));
        level->addWall(Point2D{static_cast<double>(i * 1000), 0},
                       Point2D{static_cast<double>((i + 1) * 1000), 0}, 150.0);
        tm.commitTransaction(project);
    }

    assert(tm.undoStack().size() == 3);
    assert(tm.undoStack()[0].intent == "Action 3");
    assert(tm.undoStack()[1].intent == "Action 4");
    assert(tm.undoStack()[2].intent == "Action 5");

    std::cout << "  -> Passed max history truncation." << std::endl;
}

int main() {
    std::cout << "=== KaLara Arch: History & Transaction Tests ===" << std::endl;

    testTransactionLifecycle();
    testSemanticGroupedOperationsAI();
    testRollbackOnCancel();
    testRedoStackInvalidation();
    testTransactionScopeRAII();
    testMaxHistoryLimit();

    std::cout << "=== All History & Transaction Tests Passed Successfully! ===" << std::endl;
    return 0;
}
