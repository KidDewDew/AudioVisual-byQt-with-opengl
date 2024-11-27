import QtQuick 2.7
import QtQuick.Window 2.8
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import Spectrum3D.Charts 1.0

Window {
    id: window
    width: 1280
    height: 560
    visible: true
    title: "音频播放器"
    property int fts: Math.max(width,height)*0.04
    property var volume_points:[]
    property real h_scale: 1.0
    property bool fromMacro: false
    signal isPlaying()
    Component.onCompleted: {

    }

    function selectAudioFile() {
        var d = file_dialog.createObject(window)
        d.open()
        d.onAccepted.connect(function(){
            player.setSource(d.fileUrl)
            player.play();
            play_button.selected = true
            chart.visible = true
            fromMacro = false
        })
    }
    function updateVolumeBars(volumes) {
        volume_bar.volumes = volumes
        if(volume_points.length !== volumes.length) {
            volume_points = []
            for(let i = 0; i < volumes.length; ++i) volume_points.push([])
        }
        for(let i = 0; i < volumes.length; ++i) {
            volume_points[i].push([1.0,volumes[i]])
        }
    }
    function updateSpectrumBars(spectrums) {
        //console.log(spectrums)
        spectrum_bar.new_spectrums = spectrums
        if(spectrum_bar.spectrums.length !== spectrums.length)
            spectrum_bar.spectrums = spectrums
    }
    function msToTime(ms) {
        var hours = Math.floor(ms / 3600000);
        ms -= hours * 3600000;
        var minutes = Math.floor(ms / 60000);
        ms -= minutes * 60000;
        var seconds = Math.floor(ms / 1000);
        return hours + ":" + (minutes < 10 ? "0" + minutes : minutes) + ":" + (seconds < 10 ? "0" + seconds : seconds);
    }
    Component{
        id: about_dialog
        MessageDialog {
            title:"关于该软件";
            text: "该软件用于可视化音频，采用Qt制作，制作者：刘硕、孙王毅。"
        }
    }
    Component{
        id: file_dialog
        FileDialog {}
    }

    ColumnLayout {
        anchors.fill: parent
        MenuBar {
            Layout.fillWidth: true
            background: Rectangle {
                //color: "#FFC8C9"
                color: "#F57173"
            }
            Menu {
                title: "<font color='white'>文件</font>"

                MenuItem {
                    text: "打开音频文件..."
                    onTriggered: selectAudioFile()
                }
                Menu {
                    title: "打开网络上的音频文件..."
                    //onTriggered: selectAudioFile()
                    ColumnLayout {
                        height: 150
                        Text {
                            text: "请输入Url:"
                        }
                        TextField {
                            id: tf_url
                            Layout.leftMargin: 5
                            Layout.rightMargin: 5
                            Layout.fillWidth: true
                            text: "https://ws6.stream.qqmusic.qq.com/C400003YPGGD185MWv.m4a?guid=3245483416&vkey=5BC57C316DC7FC043A5CF1956D8FC0645C8682BDE351FA13962F899F4F293D6684B6448F9C6B28D95970504F7B2CEEE9607C0EC767D7D22A&uin=3480941457&fromtag=120032"
                        }
                        Button {
                            Layout.alignment: Qt.AlignHCenter
                            text: "确定"
                            onClicked: {
                                player.setSource(tf_url.text)
                                player.play();
                                play_button.selected = true
                                chart.visible = true
                                fromMacro = false
                            }
                        }
                    }
                }
                MenuItem{
                    text: "从麦克风输入"
                    onTriggered: {
                        player.setSourceSysOInput();
                        play_button.selected = true
                        chart.visible = true
                        fromMacro = true
                        player.stop()
                    }
                }
                MenuItem {
                    text: "关闭当前音频文件"
                    onTriggered: {
                        player.stop()
                        player.setSource("")
                        chart.visible = false
                        fromMacro = false
                    }
                }
            }
            Menu {
                title: "<font color='white'>帮助</font>"
                MenuItem {
                    text: "关于..."
                    onTriggered: about_dialog.createObject(window).open()
                }
            }
            Menu {
                title: "<font color='white'>设置</font>"
                ColumnLayout {
                    Text {
                        text:"调整图表增量"
                    }
                    Slider {
                        id: slider_hScale
                        from: 0.4
                        to: 2.0
                        value: 1.0
                        onValueChanged: h_scale = value
                    }
                }
                ColumnLayout {
                    Text {
                        text:"调整频谱显示宽度"
                    }
                    Slider {
                        from: 1.0
                        to: 30.0
                        value: 6.0
                        onValueChanged: spectrum_bar.xscale = value
                    }
                }
                Switch {
                    text: "GL视图.旋转"
                    onPositionChanged: {
                        spectrum3D.setSpining(position > 0.5)
                    }
                }
                Switch {
                    text: "GL视图.开灯/关灯"
                    onPositionChanged: {
                        spectrum3D.setOffOn(position)
                    }
                }
                Switch {
                    text: "GL视图.光照"
                    checked: true
                    onPositionChanged: {
                        spectrum3D.enable_light = checked;
                    }
                }
                Switch {
                    text: "GL视图.颜色"
                    checked: false
                    onPositionChanged: {
                        spectrum3D.colorMode = checked;
                    }
                }
                Text {
                    text: "GL视图.俯瞰角"
                }
                Slider {
                    from: 0.0
                    to: 60.0
                    value: 20.0
                    onPositionChanged: {
                        spectrum3D.hoverAngle = value;
                    }
                }
                Text {
                    text: "刷新间隔"
                }
                Slider {
                    from: 20.0
                    to: 150.0
                    value: 75.0
                    onPositionChanged: {
                        timer.interval = value
                    }
                }
            }
        }

        RowLayout {
            id: chart
            visible: false
            Layout.fillHeight: true
            Layout.fillWidth: true
            Canvas {
                id: volume_bar
                Layout.alignment: Qt.AlignBottom
                property var volumes:[0.4,0.8]
                property var current_volumes: [0.0,0.0]
                Layout.preferredWidth:  fts*3
                Layout.preferredHeight: window.height * 0.8
                function run() {
                    for(let i = 0; i < volumes.length; ++i) {
                        current_volumes[i] = (volumes[i] + current_volumes[i])/2
                    }
                    volume_bar.requestPaint()
                }
                onPaint: {
                    var ctx = getContext('2d')
                    var n = volumes.length
                    ctx.clearRect(0,0,width,height)
                    var grd = ctx.createLinearGradient(0, height, 0, 0);
                    grd.addColorStop(0, '#FFDACF')
                    grd.addColorStop(0.45, '#FF4D4D')
                    grd.addColorStop(1, '#FFFC36')
                    ctx.fillStyle = grd;
                    for(let i = 0; i < n; ++i) {
                        var h = current_volumes[i]*height
                        ctx.fillRect(width*i/n,height - h,
                                     width*(i+1)/n-2,height)
                    }
                }
            }
            SplitView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                handle: Rectangle {
                    id: handleDelegate
                    implicitWidth: 4
                    implicitHeight: 4
                    color: SplitHandle.pressed ? "#FF6164"
                        : (SplitHandle.hovered ? Qt.lighter("#FCC7C8", 1.1) : "#FCC7C8")
                }

                Canvas {
                    id: spectrum_bar
                    property real xscale: 6.0
                    property var spectrums:[]
                    property var new_spectrums: []
                    property var falls: []
                    property var fall_speeds: []
                    property var n_colors: [['#2052EDAC','#FF135EFF'],['#96EDD576','#FFFF0004']]
                    property var n_colors2: ['#21FF02','#FF3C9F','#2707FF']
                    property real speed: 0.01
                    property real moving_speed: 0.01
                    property real fall_speed: 0.002
                    property real fall_speed_add: 0.0004
                    SplitView.preferredWidth:  window.width * 0.5
                    SplitView.preferredHeight: window.height * 0.7
                    //SplitView.alignment: Qt.AlignBottom
                    function run() {
                        if(new_spectrums.length == 0) return;
                        if(falls.length < new_spectrums[0].length) {
                            falls = new Array(new_spectrums[0].length).fill(0)
                            fall_speeds = new Array(falls.length).fill(fall_speed)
                        }

                        var maxVs = new Array(falls.length).fill(0)  //存储每帧最大幅值

                        for(let c = 0; c < spectrums.length; ++c) {
                            if(new_spectrums[c].length > spectrums[c].length) {
                                for(let i = spectrums[c].length; i < new_spectrums[c].length; ++i)
                                    spectrums[c].push(0)
                            } else if(new_spectrums[c].length < spectrums[c].length) {
                                spectrums[c] = spectrums[c].splice(0,new_spectrums[c].length)
                            }
                            for(let i = 0;i < spectrums[c].length; ++i) {
                                var dis = Math.abs(new_spectrums[c][i] - spectrums[c][i])
                                if(dis < speed) {
                                    spectrums[c][i] = (spectrums[c][i] + new_spectrums[c][i])/2
                                    maxVs[i] = Math.max(maxVs[i],spectrums[c][i])
                                    continue
                                }
                                if(new_spectrums[c][i] > spectrums[c][i])
                                    spectrums[c][i] += speed
                                else spectrums[c][i] -= speed
                                maxVs[i] = Math.max(maxVs[i],spectrums[c][i])
                            }
                        }

                        for(let i = 0; i < falls.length; ++i) {
                            falls[i] = Math.max(maxVs[i],falls[i]-fall_speeds[i])
                            if(falls[i] > maxVs[i])
                                fall_speeds[i] += fall_speed_add
                            else fall_speeds[i] = -fall_speed/2
                        }

                        for(let c = 0; c < volume_points.length; ++c) {
                            for(let i = 0; i < volume_points[c].length; ++i) {
                                volume_points[c][i][0] -= moving_speed
                            }
                            if(volume_points[c].length > 1 && volume_points[c][1][0] < 0.0) {
                                volume_points[c].shift() //删除最左边出界的点
                            }
                        }
                        requestPaint()
                        spectrum3D.update()
                    }
                    onPaint: {
                        var ctx = getContext('2d')
                        var n = spectrums.length  //声道数
                        if(n <= 0) return;
                        var m = spectrums[0].length / xscale
                        ctx.clearRect(0,0,width,height)
                        for(let i = 0; i < n; ++i) { //绘制时域折线图
                            ctx.strokeStyle = n_colors2[i%n_colors2.length]
                            let pn = volume_points.length
                            let H = height * 0.62, W = width
                            ctx.beginPath()
                            ctx.moveTo(0,H)
                            var endX,endY
                            for(let j = 0; j < volume_points[i].length; ++j) {
                                var [x,y]=volume_points[i][j]
                                var next_x=x,next_y=y
                                if(j + 1 < volume_points[i].length){
                                    [next_x,next_y] = volume_points[i][j+1]
                                }
                                y *= h_scale
                                next_y *= h_scale
                                x = x*W; y = H-y*H
                                endX = x; endY = y
                                next_x = next_x*W; next_y = H-next_y*H;
                                ctx.quadraticCurveTo(x,y,(next_x+x)/2,(next_y+y)/2)
                            }
                            ctx.lineTo(endX,H)
                            ctx.closePath()
                            var grd = ctx.createLinearGradient(0, H, 0, 0);
                            grd.addColorStop(0.1, Qt.rgba(1,1,1,0))
                            //grd.addColorStop(0.5, Qt.rgba(1,1,1,0))
                            grd.addColorStop(1, n_colors2[i%n_colors2.length])
                            ctx.fillStyle = grd;
                            ctx.fill()

                        }
                        for(let i = 0; i < n; ++i) {  //绘制频谱
                            var grd = ctx.createLinearGradient(0, height, 0, 0);
                            grd.addColorStop(0, n_colors[i%n_colors.length][0])
                            grd.addColorStop(1, n_colors[i%n_colors.length][1])
                            ctx.fillStyle = grd;
                            for(let j = 0; j < m; ++j) {
                                var h = spectrums[i][j]*height*11.5*h_scale
                                ctx.fillRect(width*j/m,height - h,width/m,h)
                            }
                        }
                        for(let j = 0; j < m; ++j) {  //绘制fall掉落块
                            var rV = falls[j]*11.5*h_scale
                            var addW = (width/m) * rV * rV
                            ctx.fillStyle = Qt.rgba(Math.sqrt(rV)-j/m,Math.abs(rV/3.0)+j/m,0.5+rV/2.0,Math.sqrt(rV))
                            ctx.fillRect(width*j/m-addW/2,height*(1.0 - rV),width/m+addW,(1-(1-rV)*(1-rV))*18)
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        // onWheel: {
                        //     parent.xscale += wheel.angleDelta.y/180.0
                        //     if(parent.xscale > 30.0) parent.xscale = 30.0
                        //     if(parent.xscale < 1.0) parent.xscale = 1.0
                        // }
                    }
                }
                Spectrum3D {
                    id: spectrum3D
                    SplitView.preferredWidth: height
                    SplitView.preferredHeight: window.height * 0.75
                    data: spectrum_bar.spectrums
                    h_scale: window.h_scale
                }
            }
        }
        Label {
            visible: ! chart.visible
            text: "点击打开音频...<br>&nbsp;&nbsp;或者拖动音频到此"
            font.pixelSize: fts * 1.2
            color:'#D4D4D4'
            Layout.fillHeight: true
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            MouseArea {
                anchors.fill: parent
                onClicked: selectAudioFile()
            }
            Drag.supportedActions: Qt.CopyAction
            DropArea {
                anchors.fill: parent
                onDropped: {
                    console.log(drop.urls[0])
                    player.setSource(drop.urls[0])
                    player.play();
                    play_button.selected = true
                    chart.visible = true
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            ImageButton {
                id: play_button
                selectable: true
                Layout.preferredWidth: Math.min(fts * 1.4,55)
                Layout.preferredHeight: Layout.preferredWidth
                unselected_source: "qrc:/image/play.png"
                selected_source: "qrc:/image/pause.png"
                onSelectedChanged: {
                    if(selected) player.play()
                    else player.pause()
                }
            }
            Slider {
                id: slider
                Layout.preferredWidth: window.width * 0.6
                leftPadding: 0
                rightPadding: 0
                background: Rectangle{
                    anchors.verticalCenter: slider.verticalCenter
                    radius: 5
                    height: 12
                    color: "#FFC8C9"
                    Rectangle {
                        width: slider.width * slider.value
                        radius: 5
                        height: 12
                        color: "#DB0773"
                    }
                }
                Label {  //游标
                    id: floating_label
                    color: "white"
                    font.pixelSize: fts * 0.6
                    y: -fts * 0.2
                    Behavior on opacity {
                        NumberAnimation { duration: 400 }
                    }
                    background: Rectangle {
                        color: "#DB0773"
                        radius: 5
                    }
                }
                MouseArea {
                    id: ma
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: floating_label.opacity = 1.0
                    onExited:floating_label.opacity = 0.0
                    onPositionChanged: function(mouse) {
                        mouse.accepted = false
                        if(floating_label.opacity < 0.01) return;
                        var ms = Math.round(mouse.x / ma.width * player.duration)
                        floating_label.text = msToTime(ms)
                        floating_label.x = mouse.x - floating_label.width / 2
                    }
                    onClicked: function(mouse) {
                        slider.value = mouse.x / ma.width
                        player.setPosition(slider.value*player.duration)
                        if(! player.playing()) {
                            play_button.selected = true
                            player.play()
                        }
                    }
                }
            }
            Text {
                id: time_text
                text: "0:00:00/0:00:00"
                font.pixelSize: fts * 0.6
                color: "#DB0773"
            }
            ImageButton {
                id: volume_button
                unselected_source: "qrc:/image/volume.png"
                Layout.leftMargin: fts*0.4
                Layout.preferredWidth: play_button.width * 0.8
                Layout.preferredHeight: play_button.height * 0.8
                Layout.alignment: Qt.AlignVCenter
                onClicked: {
                    popup_volume.open()
                }
                Popup {
                    id: popup_volume
                    background: Rectangle {
                        border.color: "#991F44"
                        radius: 6
                    }
                    y: -height+parent.height/2
                    x: -width/3
                    ColumnLayout {
                        Slider {
                            id: volume_slider
                            Layout.alignment: Qt.AlignHCenter
                            from:0.0
                            to:1.0
                            value: 0.5
                            orientation: Qt.Vertical
                            Component.onCompleted: player.setVolume(50)
                            onValueChanged: {
                                player.setVolume(value*100)
                            }
                        }
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/image/volume.png"
                            Layout.preferredWidth: play_button.width * 0.8
                            Layout.preferredHeight: play_button.height * 0.8
                        }
                    }
                }
            }
        }
        Item { Layout.preferredHeight: 6}
    }
    Timer {
        id: timer
        onTriggered: {
            if(player.playing()) {
                if(! slider.pressed)
                    slider.value = player.position / player.duration
                time_text.text = msToTime(player.position) + '/' + msToTime(player.duration)
                volume_bar.run()
                spectrum_bar.run()
            }
            else if(fromMacro) {
                volume_bar.run()
                spectrum_bar.run()
            }
        }
        interval: 35
        running: true
        repeat: true
    }
}
