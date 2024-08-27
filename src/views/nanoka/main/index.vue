<template>
  <div style="display:flex;">
    <el-card class="box-card" style="margin:20px; width:60%">
      <div ref="lineChart" style="height: 400px;"></div>
    </el-card>

    <el-card class="box-card" style="margin:20px; width:20%; display:flex; flex-direction:column; justify-content: center; align-items:center;">
      <template slot="header"><strong>请求成功率</strong></template>
      <div ref="progressChart" style="height: 200px; width: 200px; margin: auto;padding-top:30px;"></div>
      <p style="padding-top:30px;padding-left:25%;">成功率: <strong>{{successPercentage}}%</strong></p>
    </el-card>
  </div>
</template>

<script>
import * as echarts from 'echarts'
import axios from 'axios'
export default {
  data () {
    return {
      successPercentage: 0,
      lineChart: null,
      lineOption: null,
      progressChart: null,
      progressOption: null
    }
  },
  mounted () {
    this.fetchInfo()
    this.initLineChart()
    this.initProgressChart()
  },
  methods: {
    async fetchInfo () {
      try {
        const response = await axios.get('http://127.0.0.1:5000/info')

        if (response.status === 200) {
          this.successPercentage = response.data.info

          this.progressOption.series[0].data = [
            { value: this.successPercentage, name: 'Success', itemStyle: { color: '#42b983' } },
            { value: 100 - this.successPercentage, name: 'Failed', itemStyle: { color: '#ff4d4f' } }
          ]
          this.progressChart.setOption(this.progressOption)

          this.lineOption.xAxis.data = response.data.time_str
          this.lineOption.series[0].data = response.data.x
          this.lineChart.setOption(this.lineOption)

          this.$notify({
            title: 'Success',
            message: '获取数据成功'
          })
        } else {
          this.$notify({
            title: 'Failed',
            message: '获取数据失败'
          })
        }
      } catch (error) {
        this.$notify({
          title: 'Fatal error',
          message: '请联系工作人员解决'
        })
      }
    },
    initLineChart () {
      this.lineChart = echarts.init(this.$refs.lineChart)
      this.lineOption = {
        title: {
          text: '24小时 请求次数'
        },
        xAxis: {
          type: 'category',
          data: ['00:00', '01:00', '02:00', '03:00', '04:00', '05:00', '06:00', '07:00', '08:00', '09:00', '10:00', '11:00', '12:00', '13:00', '14:00', '15:00', '16:00', '17:00', '18:00', '19:00', '20:00', '21:00', '22:00', '23:00']
        },
        yAxis: {
          type: 'value'
        },
        series: [{
          data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
          type: 'line'
        }]
      }
      this.lineChart.setOption(this.lineOption)
    },
    initProgressChart () {
      this.progressChart = echarts.init(this.$refs.progressChart)
      this.progressOption = {
        series: [{
          type: 'pie',
          radius: ['70%', '90%'],
          avoidLabelOverlap: false,
          label: {
            show: false,
            position: 'center'
          },
          emphasis: {
            label: {
              show: true,
              fontSize: '30',
              fontWeight: 'bold'
            }
          },
          labelLine: {
            show: false
          },
          data: [
            { value: this.successPercentage, name: 'Success', itemStyle: { color: '#42b983' } },
            { value: 100 - this.successPercentage, name: 'Failed', itemStyle: { color: '#ff4d4f' } }
          ]
        }]
      }
      this.progressChart.setOption(this.progressOption)
    }
  }
}
</script>

<style scoped>
.box-card {
  margin: 20px;
}
</style>
