<template>
  <d2-container type="card">
    <template slot="header"><p><strong>静态地图分析</strong>, 将图片上传后进行解析</p></template>
    <div style="height: 500px; margin: -16px;">
      <SplitPane :min-percent='20' :default-percent='60' split="vertical">
        <template slot="paneL">
          <SplitPane :min-percent='20' :default-percent='60' split="horizontal">
            <template slot="paneL">
              <el-card class="box-card" style="margin:20px;">
                <template slot="header">静态地图上传</template>
                <el-upload
                  action="http://127.0.0.1:5000/fcn"
                  list-type="picture-card"
                  name="image"
                  :file-list="fileList"
                  :on-success="handleSuccess"
                  :on-error="handleError">
                  <i class="el-icon-plus"></i>
                </el-upload>
              </el-card>
            </template>
            <template slot="paneR">
              <el-card class="box-card" style="margin:20px;height: 160px;">
                <div class="scrollable-content">
                  <ul>
                    <li v-for="(item, index) in boxes" :key="index">
                      <strong>{{types[index]}}</strong> 坐标为:<strong>({{ item[0] }}, {{ item[1] }})</strong>  <strong>({{ item[0] + item[2] }}, {{ item[1] + item[3] }})</strong>
                    </li>
                  </ul>
                </div>
              </el-card>
            </template>
          </SplitPane>
        </template>
        <template slot="paneR">
          <el-card class="box-card" style="display: flex;flex-direction:column;justify-content: center;margin:20px;">
            <template slot="header">框选图片</template>
            <img style="width:350px; height:350px;" v-if="boxData" :src="boxData" alt="框选后的图片" />
          </el-card>
        </template>
      </SplitPane>
    </div>
  </d2-container>
</template>

<script>
import Vue from 'vue'
import SplitPane from 'vue-splitpane'
import axios from 'axios'
Vue.component('SplitPane', SplitPane)
export default {
  data () {
    return {
      fileList: [],
      boxData: null,
      boxes: null,
      imageUrl: 'http://127.0.0.1:5000/fcn_image',
      types: ['cross_1', 'cross_2', 'cross_3', 'cross_4', 'cross_5', 'cross_6', 'path_1', 'path_2', 'path_3', 'path_4', 'path_5', 'path_6', 'path_7']
    }
  },
  name: 'upload',
  mounted () {
    // 加载完成后显示提示
    this.showInfo()
  },
  methods: {
    async fetchImage () {
      try {
        const response = await axios.get(this.imageUrl, {
          responseType: 'blob'
        })
        const imageBlob = response.data
        this.boxData = URL.createObjectURL(imageBlob)
        this.$notify({
          title: 'Success',
          message: '请求框选图片成功'
        })
      } catch (error) {
        this.$notify({
          title: 'Failed',
          message: '请求框选图片失败'
        })
      }
    },
    handleSuccess (response, file, fileList) {
      if (fileList.length > 1) {
        this.fileList = [fileList[1]]
      }

      if (response.code === 0) {
        const info = response.info
        this.boxes = info
        this.$notify({
          title: 'Success',
          message: '请求框选信息成功'
        })
      } else {
        this.$notify({
          title: 'Failed',
          message: '请求框选信息失败'
        })
      }
      this.fetchImage()
      console.log('Upload success:', response)
    },
    handleError (err, file, fileList) {
      console.error('Upload error:', err)
    },
    showInfo () {
      this.$notify({
        title: 'Tips',
        message: '在横向或者纵向的分割线上拖拽调整分区大小'
      })
    }
  }
}
</script>

<style scoped>
.scrollable-content {
  max-height: 120px;
  overflow-y: auto;
  border: 1px solid #ccc;
  padding: 10px;
}
</style>
