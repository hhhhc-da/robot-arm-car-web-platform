<template>
    <d2-container>
      <template slot="header"><h1>多元可视化调试系统</h1></template>
      <d2-container>
        <el-card class="box-card">
          <div slot="header" class="clearfix">
            <span>主要功能介绍</span>
          </div>
          <div>
            <p>我们主要使用 <strong>核心</strong> 选项卡中的模块, 功能如下:</p>
            <p><strong>控制台</strong>: 用于查看近期的数据流</p>
            <p><strong>地图分析</strong>: 上传图片, 用于解析地图, 并框选出主要路径</p>
            <p><strong>小车跟踪</strong>: 解析小车在视频流中的相对位置</p>
            <p><strong>物体识别</strong>: 解析物块（区分颜色）、红色光点在视频流中的相对位置</p>
          </div>
        </el-card>
      </d2-container>

      <template slot="footer">
        <div style="display:flex;">
          <a href="https://github.com/d2-projects/d2-admin" target="_blank"><img src="https://raw.githubusercontent.com/d2-projects/d2-admin/master/docs/image/d2-admin@2x.png" width="200"></a>
          <h3 style="padding-left:30%">特别鸣谢: D2-Admin</h3>
        </div>
      </template>
    </d2-container>
  </template>

<script>
export default {
  name: 'README'
}
</script>
