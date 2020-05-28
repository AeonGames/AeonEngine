const fs = require('fs')
const { app, BrowserWindow } = require('electron')
const { 
  getRendererConstructorNames,
  setRenderer,
  Scene,
  Window,
} = require('AeonEngineModule.node');

app.disableHardwareAcceleration()

function createWindow () {
  // Create the browser window.
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    frameRate: 60,
    show: true,
    frame: true,
    /*transparent: true,*/
    webPreferences: {
        /*offscreen: true,*/
        nodeIntegration: true
      }
  })
  // and load the index.html of the app.
  win.loadFile('index.html')

  //win.webContents.openDevTools()

  win.webContents.on('paint', (event, dirty, image) => {
    console.log(image);
    /*
    fs.writeFile('tempimage.png', image.toPNG(), function (err) {
        if (err)
            throw err;
        console.log('It\'s saved!');
    });
    */
  })
  win.webContents.on('click', (event) => {
    console.log(event);
  });
  rendererNames = getRendererConstructorNames();
  console.log(rendererNames);
  setRenderer(rendererNames[0]);
  win["proxy"] = new Window(0,0,800,600,false);
  //win["proxy"] = new Window(win.getNativeWindowHandle());
  win["scene"] = new Scene("scenes/main.txt");
  win["proxy"].run(win["scene"]);
  win.on('close', function() {
    console.log("Window Closing");
    win["scene"] = null;
    win["proxy"] = null;
    if (global.gc) {global.gc(true);}
    else{console.log("No global.gc");}
  });
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', () => {
  // On macOS it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  // On macOS it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow()
  }
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.