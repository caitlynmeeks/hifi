

var MaterialInspector = Script.require('./materialInspector.js');
var Page = Script.require('./luci/Page.js');


function openView() {
    //window.closed.connect(function() { Script.stop(); });


    var pages = new Pages();
    function fromQml(message) {
        if (pages.open(message.method)) {
            return;
        }    
    }

    var luciWindow  
    function openLuciWindow(window) {
        if (luciWindow !== undefined) {
            activeWindow.fromQml.disconnect(fromQml);
        }
        if (window !== undefined) {
            window.fromQml.connect(fromQml);
        }
        luciWindow = window;


        var moveDebugCursor = false;
        var onMousePressEvent = function (e) {
            if (e.isMiddleButton) {
                moveDebugCursor = true;
                setDebugCursor(e.x, e.y);
            }
        };
        Controller.mousePressEvent.connect(onMousePressEvent);
    
        var onMouseReleaseEvent = function () {
            moveDebugCursor = false;
        };
        Controller.mouseReleaseEvent.connect(onMouseReleaseEvent);
    
        var onMouseMoveEvent = function (e) {
            if (moveDebugCursor) {
                setDebugCursor(e.x, e.y);
            }
        };
        Controller.mouseMoveEvent.connect(onMouseMoveEvent);
    
        function setDebugCursor(x, y) {
            var nx = 2.0 * (x / Window.innerWidth) - 1.0;
            var ny = 1.0 - 2.0 * ((y) / (Window.innerHeight));
    
            Render.getConfig("RenderMainView").getConfig("DebugDeferredBuffer").size = { x: nx, y: ny, z: 1.0, w: 1.0 };
        }

    }

    function closeLuciWindow() {
        if (luciWindow !== undefined) {
            activeWindow.fromQml.disconnect(fromQml);
        }
        luciWindow = {};

        Controller.mousePressEvent.disconnect(onMousePressEvent);
        Controller.mouseReleaseEvent.disconnect(onMouseReleaseEvent);
        Controller.mouseMoveEvent.disconnect(onMouseMoveEvent);
        pages.clear();
    }

    pages.addPage('Luci', 'Luci', '../luci.qml', 300, 420, openLuciWindow, closeLuciWindow);
    pages.addPage('openEngineInspectorView', 'Render Engine Inspector', '../engineInspector.qml', 300, 400);
    pages.addPage('openEngineLODView', 'Render LOD', '../lod.qml', 300, 400);
    pages.addPage('openMaterialInspectorView', 'Material Inspector', '../materialInspector.qml', 300, 400, MaterialInspector.setWindow, MaterialInspector.setWindow);

    pages.open('Luci');

    
    return pages;
}


openView();

