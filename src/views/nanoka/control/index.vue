<template>
    <d2-container type="card">
        <template slot="header"><p><strong>机械臂调试控制台</strong></p></template>
        <span style="font-size:smaller;">Control Type</span>
        <div>
            <el-select v-model="selectedOption" placeholder="请选择" @change="handleSelectChange">
              <el-option
                v-for="item in options"
                :key="item.value"
                :label="item.label"
                :value="item.value">
              </el-option>
            </el-select>

            <div v-for="n in paramCount" :key="n" class="param-input">
              <span style="font-size:smaller;">param {{ n }}</span>
              <el-input v-model="params[n - 1]"></el-input>
            </div>

            <div style="display:flex;flex-direction:column;align-items:center;justify-content:center;padding-top:20px;">
                <el-button type="success" style="margin:5px;width:80%;" @click="exec" round>执行操作</el-button>
                <el-button type="danger" style="margin:5px;width:80%;" @click="clear" round>清空参数</el-button>
            </div>
          </div>
      </d2-container>
  </template>

<script>
import axios from 'axios'
export default {
  data () {
    return {
      selectedOption: 'arm_move',
      options: [
        { value: 'arm_move', label: '移动机械臂', params: 3 },
        { value: 'detect_move', label: '移动定位器', params: 2 },
        { value: 'platform', label: '移动平台', params: 1 },
        { value: 'arm_transpose', label: '挪动物体', params: 6 },
        { value: 'music', label: '播放音乐', params: 0 },
        { value: 'red', label: '调节红外线发射器', params: 1 }
      ],
      selectedIndex: 0,
      paramCount: 3,
      params: []
    }
  },
  methods: {
    handleSelectChange (value) {
      this.selectedIndex = this.options.findIndex(option => option.value === value)
      this.paramCount = this.options[this.selectedIndex].params
      this.generateParams()
    },
    generateParams () {
      this.params = Array.from({ length: this.paramCount }, () => '')
    },
    exec () {
      // {"cmd":"platform","params_num":1, "params":[45]}
      var data = ''
      if (this.paramCount === 0) {
        data = '{"cmd":"' + this.selectedOption + '","params_num":' + this.paramCount + '}'
      } else {
        data = '{"cmd":"' + this.selectedOption + '","params_num":' + this.paramCount + ',"params":[' + this.params + ']}'
      }
      console.log(data)
      //   var data = JSON.stringify({
      //     cmd: this.selectedOption,
      //     params_num: this.paramCount,
      //     params: this.params
      //   })
      axios.post('http://127.0.0.1:5000/arm_exec', data, {
        headers: {
          'Content-Type': 'text/plain'
        //   'Content-Type': 'application/json'
        }
      })
        .then(response => {
          this.$notify({
            title: 'Response',
            message: response.data
          })
        })
        .catch(error => {
          this.$notify({
            title: 'Error',
            message: error
          })
        })
    },
    clear () {
      this.generateParams()
    }
  },
  mounted () {
    this.generateParams()
  }
}
</script>

  <style scoped>
  .param-input {
    margin-top: 10px;
  }
  </style>
