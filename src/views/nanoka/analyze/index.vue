<template>
  <d2-container type="full" class="page">
    <d2-grid-layout
      v-bind="layout"
      @layout-updated="layoutUpdatedHandler">
      <d2-grid-item
        v-for="(item, index) in layout.layout"
        :key="index"
        v-bind="item"
        @resize="resizeHandler"
        @move="moveHandler"
        @resized="resizedHandler"
        @moved="movedHandler">
        <el-card shadow="never" class="page_card">
          <el-tag size="mini" type="info" slot="header">{{item.i}}</el-tag>
          <template v-if="item.i === '当前图片'">
            <el-card class="box-card" style="display: flex;justify-content: center;">
              <p><strong>上一次</strong>保存的地图</p>
            </el-card>
            <el-card class="box-card" style="display: flex;justify-content: center;">
              <img style="width:300px; height:300px;" v-if="saveData" :src="saveData" alt="保存展示的图片" />
            </el-card>
          </template>
          <template v-if="item.i === '视频流'">
            <el-card class="box-card" style="display: flex;justify-content: center;">
              <p><strong>即时</strong>视频流展示</p>
            </el-card>
            <el-card class="box-card" style="display: flex;justify-content: center;">
              <img style="width:300px; height:300px;" v-if="streamData" :src="streamData" alt="从后端请求的图片" />
            </el-card>
          </template>
          <template v-if="item.i === '操作按钮'">
            <el-card class="box-card">
              <div style="display:flex;flex-direction:column;align-items:center;justify-content:center;">
                <el-button type="primary" style="margin:5px;width:80%;" @click="stream_on" round>请求视频流</el-button>
                <el-button type="warning" style="margin:5px;width:80%;" @click="stream_off" round>终止视频流</el-button>
                <el-button type="success" style="margin:5px;width:80%;" @click="save_image" round>保存视频帧</el-button>
                <el-button type="danger" style="margin:5px;width:80%;" @click="clear_image" round>清除视频帧</el-button>
                <el-button type="danger" style="margin:5px;width:80%;" @click="clear_note" round>清除笔记</el-button>
              </div>
            </el-card>
          </template>
          <template v-if="item.i === 'NoteBook'">
            <el-input type="textarea" v-model="noteContent" placeholder="在这里输入你的笔记" :rows="2"></el-input>
          </template>
        </el-card>
      </d2-grid-item>
    </d2-grid-layout>
  </d2-container>
</template>

<script>
import Vue from 'vue'
import { GridLayout, GridItem } from 'vue-grid-layout'
import axios from 'axios'
Vue.component('d2-grid-layout', GridLayout)
Vue.component('d2-grid-item', GridItem)
export default {
  data () {
    return {
      layout: {
        layout: [
          { x: 0, y: 0, w: 4, h: 14, i: '当前图片' },
          { x: 4, y: 4, w: 4, h: 14, i: '视频流' },
          { x: 8, y: 4, w: 4, h: 10, i: '操作按钮' },
          { x: 8, y: 0, w: 4, h: 4, i: 'NoteBook' }
        ],
        colNum: 12,
        rowHeight: 30,
        isDraggable: true,
        isResizable: true,
        isMirrored: false,
        verticalCompact: true,
        margin: [10, 10],
        useCssTransforms: true
      },
      noteContent: '',
      streamData: null,
      saveData: null,
      interval: null,
      stream_status: '0'
    }
  },
  mounted () {
    this.requestResult()
    this.showInfo()
  },
  beforeDestroy () {
    if (this.stream_status === '1') {
      clearInterval(this.interval)
    } else {
      this.stream_status = '0'
    }
  },
  methods: {
    async requestResult () {
      try {
        const response = await axios.get('http://127.0.0.1:5000/yolo')
        this.stream_pos = response.data.info.point
      } catch (error) {
        console.error('获取图片失败:', error)
      }
    },
    async fetchImage () {
      try {
        const response = await axios.get('http://127.0.0.1:5000/yolo_image', {
          responseType: 'blob'
        })
        const imageBlob = response.data
        this.streamData = URL.createObjectURL(imageBlob)
        this.requestResult()
      } catch (error) {
        this.requestResult()
        console.error('获取图片失败:', error)
      }
    },
    log (arg1 = 'log', ...logs) {
      if (logs.length === 0) {
        console.log(arg1)
      } else {
        console.group(arg1)
        logs.forEach(e => {
          console.log(e)
        })
        console.groupEnd()
      }
    },
    // 显示提示
    showInfo () {
      this.$notify({
        title: 'Tips',
        message: '你可以按住卡片拖拽改变位置；或者在每个卡片的右下角拖动，调整卡片大小'
      })
    },
    // 测试代码
    layoutUpdatedHandler (newLayout) {
      console.group('layoutUpdatedHandler')
      newLayout.forEach(e => {
        console.log(`{'x': ${e.x}, 'y': ${e.y}, 'w': ${e.w}, 'h': ${e.h}, 'i': '${e.i}'},`)
      })
      console.groupEnd()
    },
    resizeHandler (i, newH, newW) {
      this.log('resizeHandler', `i: ${i}, newH: ${newH}, newW: ${newW}`)
    },
    moveHandler (i, newX, newY) {
      this.log('moveHandler', `i: ${i}, newX: ${newX}, newY: ${newY}`)
    },
    resizedHandler (i, newH, newW, newHPx, newWPx) {
      this.log('resizedHandler', `i: ${i}, newH: ${newH}, newW: ${newW}, newHPx: ${newHPx}, newWPx: ${newWPx}`)
      if (i === 'NoteBook') {
        this.notebook_rows = newH - 2
      }
    },
    movedHandler (i, newX, newY) {
      this.log('movedHandler', `i: ${i}, newX: ${newX}, newY: ${newY}`)
    },
    // 开启视频流
    stream_on () {
      if (this.stream_status === '0') {
        this.stream_status = '1'
        this.interval = setInterval(this.fetchImage, 1000)

        this.$notify({
          title: 'Success',
          message: '开始请求视频流'
        })
      } else {
        this.$notify({
          title: 'Failed',
          message: '视频流处于打开状态'
        })
      }
    },
    stream_off () {
      if (this.stream_status === '1') {
        this.stream_status = '0'
        this.streamData = null
        clearInterval(this.interval)

        this.$notify({
          title: 'Success',
          message: '已关闭视频流'
        })
      } else {
        this.$notify({
          title: 'Failed',
          message: '视频流处于关闭状态'
        })
      }
    },
    save_image () {
      if (this.streamData != null) {
        this.saveData = this.streamData

        this.$notify({
          title: 'Success',
          message: '视频帧保存成功'
        })
      } else {
        this.$notify({
          title: 'Failed',
          message: '视频帧保存失败'
        })
      }
    },
    clear_image () {
      if (this.saveData != null) {
        this.saveData = null

        this.$notify({
          title: 'Success',
          message: '已清除视频帧'
        })
      } else {
        this.$notify({
          title: 'Failed',
          message: '无有效视频帧'
        })
      }
    },
    clear_note () {
      this.noteContent = ''

      this.$notify({
        title: 'Success',
        message: '已完成笔记清除'
      })
    }
  }
}
</script>

<style lang="scss" scoped>
.page {
  .vue-grid-layout {
    background-color: $color-bg;
    border-radius: 4px;
    margin: -10px;
    .page_card {
      height: 100%;
      @extend %unable-select;
    }
    .vue-resizable-handle {
      opacity: .3;
      &:hover{
        opacity: 1;
      }
    }
  }
}
</style>
